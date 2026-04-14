import { Injectable, Logger, OnModuleInit } from '@nestjs/common';
import { PrismaService } from './prisma.service';

// --- Levenshtein distance ---
function levenshtein(a: string, b: string): number {
  const m = a.length, n = b.length;
  if (m === 0) return n;
  if (n === 0) return m;
  const d: number[][] = Array.from({ length: m + 1 }, () => Array(n + 1).fill(0));
  for (let i = 0; i <= m; i++) d[i][0] = i;
  for (let j = 0; j <= n; j++) d[0][j] = j;
  for (let i = 1; i <= m; i++)
    for (let j = 1; j <= n; j++)
      d[i][j] = Math.min(d[i - 1][j] + 1, d[i][j - 1] + 1, d[i - 1][j - 1] + (a[i - 1] !== b[j - 1] ? 1 : 0));
  return d[m][n];
}

function normalize(s: string): string {
  return s.toLowerCase().normalize('NFD').replace(/[\u0300-\u036f]/g, '').trim();
}

// --- Intent config ---
interface IntentRule {
  required: string[];   // at least 1 must appear
  boost: string[];      // at least 1 must appear
  maxDistance: number;   // max words between required and boost terms
}

interface TriggerRule {
  terms: RegExp;
  category: string;
  priority: string;
}

interface PersonCache {
  id: string;
  name: string;
  normalized: string;
  profileSlug?: string;
}

@Injectable()
export class MatcherService implements OnModuleInit {
  private readonly logger = new Logger('Matcher');
  private persons: PersonCache[] = [];
  private directors: PersonCache[] = [];

  // --- Intent rules ---
  private intents: Record<string, IntentRule> = {
    'privacy.activate': {
      required: ['privad', 'privacidade', 'incognito', 'incógnito', 'sandbox'],
      boost: ['ativ', 'abre', 'entr', 'liga', 'modo', 'iniciar'],
      maxDistance: 5,
    },
    'privacy.deactivate': {
      required: ['privad', 'privacidade', 'privado'],
      boost: ['fecha', 'desativ', 'sai', 'encerr', 'fechar', 'sair', 'desliga', 'para'],
      maxDistance: 5,
    },
    'simulation.exit': {
      required: ['simulaç', 'simula', 'fingir', 'fingindo'],
      boost: ['sai', 'para', 'encerr', 'fecha', 'volta', 'pode parar', 'sair'],
      maxDistance: 5,
    },
    'specialist.exit': {
      required: [],
      boost: ['fecha', 'obrigado técnico', 'pode fechar', 'close'],
      maxDistance: 0, // any single boost term matches
    },
  };

  // --- Repo rules ---
  private repoRules: Record<string, string[]> = {
    'patria-api': ['endpoint', 'backend', 'prisma', 'migration', 'controller', 'service', 'nestjs', 'dto', 'api', 'webhook', 'cron'],
    'patria-app': ['frontend', 'componente', 'tela', 'dashboard', 'multi-tenant', 'tenant', 'react', 'formulário', 'formulario', 'botão', 'botao', 'app', 'página', 'pagina'],
    'landpage': ['landpage', 'landing', 'institucional', 'seção', 'secao', 'css'],
  };

  // --- Memory triggers per profile ---
  private memoryTriggers: Record<string, TriggerRule[]> = {
    gestora: [
      { terms: /\b(decid|decidimos|decidiu|foco|prioridade|estratégia|objetivo|meta|prazo|deadline)\b/i, category: 'decision', priority: 'long_term' },
      { terms: /\b(prefir|prefere|gosto|não gosto|sempre|nunca faça|evite)\b/i, category: 'preference', priority: 'long_term' },
      { terms: /\b(concluí|finalizou|lançou|deployou|promoveu|migrou|implementou)\b/i, category: 'project', priority: 'short_term' },
      { terms: /\b(bug|erro|problema|falha|quebrou|caiu|travou)\b/i, category: 'technical', priority: 'short_term' },
      { terms: /\b(contrat|demit|entrou|saiu|parceiro|cliente|fornecedor)\b/i, category: 'person', priority: 'long_term' },
    ],
    amiga: [
      { terms: /\b(aniversário|nasceu|data|evento|casamento|formatura|festa)\b/i, category: 'person', priority: 'long_term' },
      { terms: /\b(gosta|adora|odeia|prefere|favorit|hobby)\b/i, category: 'preference', priority: 'long_term' },
    ],
    juridica: [
      { terms: /\b(contrato|cláusula|lei|artigo|lgpd|processo|ação judicial|multa)\b/i, category: 'decision', priority: 'long_term' },
    ],
    mentora: [
      { terms: /\b(conselho|orientação|sugestão|recomendo|aprendi|lição|insight)\b/i, category: 'decision', priority: 'long_term' },
    ],
    assistente: [
      { terms: /\b(agenda|reunião|compromisso|lembrete|prazo|data)\b/i, category: 'project', priority: 'short_term' },
    ],
  };

  constructor(private prisma: PrismaService) {}

  async onModuleInit() {
    await this.refreshCache();
  }

  async refreshCache() {
    try {
      const people = await this.prisma.$queryRawUnsafe(
        `SELECT p.id, p.name, p.role, pr.slug as profile_slug
         FROM people p LEFT JOIN profiles pr ON p.profile_id = pr.id`,
      ) as any[];
      this.persons = people.map(p => ({ id: p.id, name: p.name, normalized: normalize(p.name), profileSlug: p.profile_slug }));
      this.directors = this.persons.filter(p => p.profileSlug === 'gestora' || (p as any).role === 'director');
      this.logger.log(`Cache loaded: ${this.persons.length} persons, ${this.directors.length} directors`);
    } catch (err) {
      this.logger.error(`Cache load failed: ${err.message}`);
    }
  }

  // --- Intent matching (term proximity) ---

  matches(input: string, intentKey: string): boolean {
    const rule = this.intents[intentKey];
    if (!rule) return false;

    const lower = normalize(input);
    const words = lower.split(/\s+/);

    // Special case: no required terms, any boost matches
    if (rule.required.length === 0) {
      return rule.boost.some(b => lower.includes(normalize(b)));
    }

    // Find if at least 1 required term appears
    let requiredIdx = -1;
    let requiredFound = false;
    for (let i = 0; i < words.length; i++) {
      if (rule.required.some(r => words[i].includes(normalize(r)))) {
        requiredIdx = i;
        requiredFound = true;
        break;
      }
    }
    if (!requiredFound) return false;

    // Find if at least 1 boost term appears within maxDistance
    for (let i = 0; i < words.length; i++) {
      if (rule.boost.some(b => words[i].includes(normalize(b)))) {
        if (rule.maxDistance === 0 || Math.abs(i - requiredIdx) <= rule.maxDistance) {
          return true;
        }
      }
    }

    // Also check multi-word boost terms
    return rule.boost.some(b => {
      const nb = normalize(b);
      if (!nb.includes(' ')) return false;
      return lower.includes(nb);
    });
  }

  // --- Fuzzy person search (Levenshtein) ---

  findPerson(input: string, maxDistance = 2): PersonCache | null {
    if (!input) return null;
    const norm = normalize(input);

    // Exact match first
    const exact = this.persons.find(p => p.normalized === norm);
    if (exact) return exact;

    // Contains match
    const contains = this.persons.find(p => p.normalized.includes(norm) || norm.includes(p.normalized));
    if (contains) return contains;

    // Levenshtein fuzzy match
    let best: PersonCache | null = null;
    let bestDist = maxDistance + 1;
    for (const p of this.persons) {
      // Compare against each word of the name too
      const names = [p.normalized, ...p.normalized.split(' ')];
      for (const n of names) {
        const dist = levenshtein(norm, n);
        if (dist < bestDist) {
          bestDist = dist;
          best = p;
        }
      }
    }
    return best;
  }

  async findPersonFull(input: string): Promise<any> {
    const cached = this.findPerson(input);
    if (!cached) return null;
    return this.prisma.$queryRawUnsafe(
      `SELECT p.id, p.name, p.role, p.description, p.context,
              pr.slug as profile_slug, pr.system_prompt as profile_prompt,
              pr.allowed_tools as allowed_tools, pr.memory_access as memory_access
       FROM people p LEFT JOIN profiles pr ON p.profile_id = pr.id
       WHERE p.id = $1`,
      cached.id,
    ).then((r: any) => r[0] || null);
  }

  // --- Repo inference (weighted terms) ---

  detectRepo(text: string): string {
    const lower = normalize(text);
    let bestRepo = 'patria-api';
    let bestScore = 0;

    for (const [repo, terms] of Object.entries(this.repoRules)) {
      let score = 0;
      for (const term of terms) {
        if (lower.includes(normalize(term))) score++;
      }
      if (score > bestScore) {
        bestScore = score;
        bestRepo = repo;
      }
    }
    return bestRepo;
  }

  detectMultiRepo(text: string): string[] {
    const lower = normalize(text);
    const repos: string[] = [];
    for (const [repo, terms] of Object.entries(this.repoRules)) {
      if (terms.some(t => lower.includes(normalize(t)))) repos.push(repo);
    }
    return repos.length > 1 ? repos : [];
  }

  // --- Director mention detection ---

  isDirectorMentioned(text: string): boolean {
    const lower = normalize(text);
    return this.directors.some(d => lower.includes(d.normalized) || d.normalized.split(' ').some(n => n.length > 3 && lower.includes(n)));
  }

  // --- Memory triggers ---

  getMemoryTrigger(profileSlug: string, text: string): { category: string; priority: string } | null {
    const triggers = this.memoryTriggers[profileSlug];
    if (!triggers) return null;
    for (const t of triggers) {
      if (t.terms.test(text)) return { category: t.category, priority: t.priority };
    }
    return null;
  }
}
