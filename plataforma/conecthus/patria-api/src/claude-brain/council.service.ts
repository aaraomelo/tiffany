import { Injectable, Logger, BadRequestException } from '@nestjs/common';

const BRIDGE_URL = process.env.BRIDGE_URL || 'http://host.docker.internal:9090';
const BRIDGE_SECRET = process.env.BRIDGE_SECRET || 'wk_infer_patria_2026';

// Mapeamento de membros do conselho com domínio + auto-routing keywords
const COUNCIL: Record<string, { model: string; dominio: string; keywords: RegExp }> = {
  adao:       { model: 'multiverso-adao',       dominio: 'Linguagem & Narrativa',
                keywords: /\b(narrativ|texto|literatur|escrev|prosa|poesi|cultura)\b/i },
  lamarck:    { model: 'multiverso-lamarck',    dominio: 'Biologia evolutiva',
                keywords: /\b(biolog|evolu|herd|gen[eé]tic|dna|adapta|sele[çc]|esp[eé]ci)\b/i },
  schnorr:    { model: 'multiverso-schnorr',    dominio: 'Cripto & Segurança',
                keywords: /\b(cripto|seguran|assinatur|chave|hash|integridade|certificad|autentica)\b/i },
  chomsky:    { model: 'multiverso-chomsky',    dominio: 'Linguística',
                keywords: /\b(lingu[ií]stic|sintax|gram[aá]tic|sem[aâ]ntic|fonet|babel)\b/i },
  wildberger: { model: 'multiverso-wildberger', dominio: 'Geometria racional',
                keywords: /\b(geometri|topolog|n[uú]mero|c[íi]rcul|tri[âa]ngul|raz[ãa]o|raciona)\b/i },
};

@Injectable()
export class CouncilService {
  private readonly logger = new Logger('Council');

  /** Auto-detecta quais conselheiros chamar baseado em palavras-chave da pergunta. */
  detectMembers(question: string): string[] {
    const matches: string[] = [];
    for (const [name, cfg] of Object.entries(COUNCIL)) {
      if (cfg.keywords.test(question)) matches.push(name);
    }
    return matches;
  }

  list(): { members: { name: string; model: string; dominio: string }[] } {
    return {
      members: Object.entries(COUNCIL).map(([name, cfg]) => ({
        name, model: cfg.model, dominio: cfg.dominio,
      })),
    };
  }

  /** Consulta os membros indicados (ou auto-detecta) e retorna respostas individuais. */
  async consult(opts: {
    question: string;
    members?: string[];
    max_tokens?: number;
  }): Promise<{
    question: string;
    consulted: { name: string; dominio: string; response: string; ms: number; error?: string }[];
  }> {
    const q = (opts.question || '').trim();
    if (!q) throw new BadRequestException('question required');

    let members = opts.members && opts.members.length > 0
      ? opts.members
      : this.detectMembers(q);
    // Sem match nenhum: chama Adão como fallback (linguagem geral)
    if (members.length === 0) members = ['adao'];
    // Filtra pra só nomes válidos
    members = members.filter((m) => COUNCIL[m]);
    if (members.length === 0) {
      throw new BadRequestException('no valid council members in selection');
    }
    // Limita a 4 paralelos pra não saturar GEX44
    members = members.slice(0, 4);

    const max_tokens = Math.min(opts.max_tokens || 100, 200);
    const tasks = members.map(async (name) => {
      const cfg = COUNCIL[name];
      const t0 = Date.now();
      try {
        const r = await fetch(`${BRIDGE_URL}/llm/chat`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json', 'X-Bridge-Key': BRIDGE_SECRET },
          body: JSON.stringify({
            model: cfg.model,
            max_tokens,
            messages: [{ role: 'user', content: q }],
          }),
          signal: AbortSignal.timeout(90_000),
        });
        const data = await r.json();
        if (!r.ok) {
          return { name, dominio: cfg.dominio, response: '', ms: Date.now() - t0, error: data?.error || `${r.status}` };
        }
        const text = data?.content?.[0]?.text || '';
        return { name, dominio: cfg.dominio, response: text, ms: Date.now() - t0 };
      } catch (e: any) {
        return { name, dominio: cfg.dominio, response: '', ms: Date.now() - t0, error: e.message?.slice(0, 200) };
      }
    });
    const consulted = await Promise.all(tasks);
    return { question: q, consulted };
  }
}
