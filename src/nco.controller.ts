import { Controller, Get, Post, Body, Query, HttpException, HttpStatus, Logger, Header, Headers, Res, Req, Sse, MessageEvent as NestMessageEvent } from '@nestjs/common';
import { readFileSync } from 'fs';
import { join } from 'path';
import type { Response, Request } from 'express';
import { Observable, Subject, merge, interval, map } from 'rxjs';
import { PrismaService } from './prisma.service';

const NCO_BASE = process.env.PATRIA_NCO_URL || 'http://host.docker.internal:8001';

// SSE: empurra eventos pra browsers conectados quando bench termina.
// Singleton em memória — broadcast sem filtro por tenant (bench é público).
class BenchEventHub {
  private static subject = new Subject<NestMessageEvent>();
  static stream(): Observable<NestMessageEvent> {
    return BenchEventHub.subject.asObservable();
  }
  static emit(payload: any) {
    BenchEventHub.subject.next({
      data: JSON.stringify(payload),
      type: payload.type || 'bench-update',
    });
  }
}

let LANDING_HTML: string | null = null;
function loadLanding(): string {
  if (LANDING_HTML !== null) return LANDING_HTML;
  try {
    const p = join(__dirname, '..', 'public', 'nco-landing.html');
    LANDING_HTML = readFileSync(p, 'utf-8');
  } catch {
    LANDING_HTML = '<h1>NCO API</h1>';
  }
  return LANDING_HTML;
}

@Controller('api/nco')
export class NcoController {
  private readonly logger = new Logger(NcoController.name);
  constructor(private readonly prisma: PrismaService) {}

  @Get()
  @Header('Content-Type', 'text/html; charset=utf-8')
  async root(@Res() res: Response): Promise<void> { res.send(loadLanding()); }

  @Get('maxcut')
  @Header('Content-Type', 'text/html; charset=utf-8')
  async maxcutGet(@Res() res: Response): Promise<void> { res.send(loadLanding()); }

  @Get('coloring')
  @Header('Content-Type', 'text/html; charset=utf-8')
  async coloringGet(@Res() res: Response): Promise<void> { res.send(loadLanding()); }

  @Get('mis')
  @Header('Content-Type', 'text/html; charset=utf-8')
  async misGet(@Res() res: Response): Promise<void> { res.send(loadLanding()); }

  @Get('health')
  async health() {
    try {
      const r = await fetch(`${NCO_BASE}/api/nco/health`, { signal: AbortSignal.timeout(5000) });
      if (!r.ok) throw new Error(`upstream ${r.status}`);
      return await r.json();
    } catch (e: any) {
      throw new HttpException(
        { status: 'degraded', detail: `NCO inference service unreachable: ${e.message}` },
        HttpStatus.SERVICE_UNAVAILABLE,
      );
    }
  }

  // Cache 5min — torch.load do ckpt é caro
  private _ckptInfoCache: { ts: number; data: any } = { ts: 0, data: null };
  @Get('ckpt-info')
  async ckptInfo() {
    const now = Date.now();
    if (this._ckptInfoCache.data && now - this._ckptInfoCache.ts < 300_000) {
      return this._ckptInfoCache.data;
    }
    try {
      const r = await fetch(`${NCO_BASE}/api/nco/ckpt-info`, { signal: AbortSignal.timeout(8000) });
      if (!r.ok) throw new Error(`upstream ${r.status}`);
      const data = await r.json();
      this._ckptInfoCache = { ts: now, data };
      return data;
    } catch (e: any) {
      throw new HttpException(
        { status: 'degraded', detail: `ckpt-info unreachable: ${e.message}` },
        HttpStatus.SERVICE_UNAVAILABLE,
      );
    }
  }

  // ---------- Listing & metrics (GET, requires auth) ----------
  @Get('calls')
  async listCalls(@Req() req: Request, @Query('limit') limit = '50', @Query('endpoint') endpoint?: string) {
    const apiKey = (req.headers['x-api-key'] as string) || '';
    const lim = Math.min(parseInt(limit) || 50, 200);
    const where: any = { apiKey };
    if (endpoint) where.endpoint = endpoint;
    const calls = await this.prisma.ncoCall.findMany({
      where, orderBy: { ts: 'desc' }, take: lim,
      select: {
        id: true, ts: true, endpoint: true, n: true,
        modelName: true, rollouts: true, timeMs: true,
        statusCode: true, errorMsg: true,
      },
    });
    return calls.map(c => ({ ...c, id: c.id.toString() }));
  }

  // Stats agregadas públicas (sem PII, sem apiKey filter) — pra landpage
  private _publicStatsCache: { ts: number; data: any } = { ts: 0, data: null };
  @Get('public-stats')
  async publicStats() {
    const now = Date.now();
    if (this._publicStatsCache.data && now - this._publicStatsCache.ts < 30_000) {
      return this._publicStatsCache.data;
    }
    const since24h = new Date(now - 24 * 3600 * 1000);
    const since7d = new Date(now - 7 * 24 * 3600 * 1000);

    const [total, totalLast24h, byEp, last] = await Promise.all([
      this.prisma.ncoCall.count({ where: { statusCode: { lt: 400 } } }),
      this.prisma.ncoCall.count({ where: { statusCode: { lt: 400 }, ts: { gte: since24h } } }),
      this.prisma.ncoCall.groupBy({
        by: ['endpoint'],
        where: { statusCode: { lt: 400 }, ts: { gte: since7d } },
        _count: true,
        _avg: { timeMs: true, n: true },
      }),
      this.prisma.ncoCall.findFirst({
        where: { statusCode: { lt: 400 } },
        orderBy: { ts: 'desc' },
        select: { ts: true, endpoint: true, n: true, timeMs: true },
      }),
    ]);

    const data = {
      total_calls: total,
      calls_last_24h: totalLast24h,
      by_endpoint: byEp.map((e) => ({
        endpoint: e.endpoint,
        count: e._count,
        avg_time_ms: e._avg.timeMs ? Math.round(e._avg.timeMs * 10) / 10 : null,
        avg_n: e._avg.n ? Math.round(e._avg.n) : null,
      })),
      last_call: last
        ? { ts: last.ts, endpoint: last.endpoint, n: last.n, time_ms: last.timeMs }
        : null,
      generated_at: new Date(now).toISOString(),
    };
    this._publicStatsCache = { ts: now, data };
    return data;
  }

  // Snapshot de bench periódico (vantagem hiper vs vet) — pra tabela da landpage
  private _benchCache: { ts: number; data: any } = { ts: 0, data: null };
  @Get('scaling-bench')
  async scalingBench() {
    const now = Date.now();
    if (this._benchCache.data && now - this._benchCache.ts < 60_000) {
      return this._benchCache.data;
    }
    const latest = await this.prisma.$queryRaw<
      Array<{ ts: Date; wall_time_s: number; data: any }>
    >`SELECT ts, wall_time_s, data FROM bench_snapshots ORDER BY ts DESC LIMIT 1`;
    if (latest.length === 0) {
      const fallback = { rows: [], updated_at: null };
      this._benchCache = { ts: now, data: fallback };
      return fallback;
    }
    const row = latest[0];
    const data = {
      rows: row.data?.rows || [],
      updated_at: row.ts.toISOString(),
      wall_time_s: row.wall_time_s,
    };
    this._benchCache = { ts: now, data };
    return data;
  }

  // Evolução temporal dos 4 ckpts em treino (v4_n1k/n5k/n10k/n20k).
  // Cada daemon marca um ponto cada 1k steps; agrupados por daemon pro front.
  private _evCache: { ts: number; data: any } = { ts: 0, data: null };
  @Get('bench-evolution')
  async benchEvolution() {
    const now = Date.now();
    if (this._evCache.data && now - this._evCache.ts < 60_000) {
      return this._evCache.data;
    }
    const path = '/app/data/bench_evolution.jsonl';
    let lines: string[] = [];
    try {
      lines = readFileSync(path, 'utf-8').split('\n').filter((l) => l.trim());
    } catch {
      const fallback = { daemons: {}, updated_at: null };
      this._evCache = { ts: now, data: fallback };
      return fallback;
    }
    const points: Array<any> = lines.map((l) => {
      try { return JSON.parse(l); } catch { return null; }
    }).filter(Boolean);
    const daemons: Record<string, Array<{ step: number; n: number; cut_mean: number; ts: string }>> = {};
    for (const p of points) {
      if (!p.daemon) continue;
      if (!daemons[p.daemon]) daemons[p.daemon] = [];
      daemons[p.daemon].push({
        step: p.step,
        n: p.n,
        cut_mean: p.cut_mean,
        ts: p.ts,
      });
    }
    // Ordena por step dentro de cada daemon
    for (const k of Object.keys(daemons)) {
      daemons[k].sort((a, b) => a.step - b.step);
    }
    const last = points.length > 0 ? points[points.length - 1].ts : null;
    const data = { daemons, updated_at: last, total_points: points.length };
    this._evCache = { ts: now, data };
    return data;
  }

  // Trigger manual de bench (admin) + status público
  @Post('bench/trigger')
  async benchTrigger(@Headers() headers: Record<string, string>) {
    const adminToken = process.env.NCO_ADMIN_TOKEN || '';
    if (!adminToken) {
      throw new HttpException('Admin trigger não configurado', HttpStatus.SERVICE_UNAVAILABLE);
    }
    const sent = (headers['x-admin-token'] as string) || '';
    if (sent !== adminToken) {
      throw new HttpException('Token admin inválido', HttpStatus.UNAUTHORIZED);
    }
    const fs = require('fs');
    const flagPath = '/app/data/bench_trigger.flag';
    fs.writeFileSync(flagPath, new Date().toISOString());
    this._evCache = { ts: 0, data: null };
    return { ok: true, flagged_at: new Date().toISOString(),
             message: 'trigger enfileirado, próxima execução em até 60s' };
  }

  @Get('bench/status')
  async benchStatus() {
    const fs = require('fs');
    const path = '/app/data/bench_evolution.jsonl';
    const triggerPath = '/app/data/bench_last_trigger.txt';
    let pointsTotal = 0;
    let lastUpdate: string | null = null;
    let lastTrigger: string | null = null;
    try {
      const stat = fs.statSync(path);
      lastUpdate = stat.mtime.toISOString();
      const content = fs.readFileSync(path, 'utf-8');
      pointsTotal = content.split('\n').filter((l: string) => l.trim()).length;
    } catch {}
    try {
      lastTrigger = fs.readFileSync(triggerPath, 'utf-8').trim();
    } catch {}
    return {
      jsonl_last_update: lastUpdate,
      points_total: pointsTotal,
      last_manual_trigger: lastTrigger,
      cron_interval_min: 5,
      note: 'cron auto a cada 5min · trigger manual via POST /api/nco/bench/trigger',
    };
  }

  // SSE público: browsers escutam pra atualizar gráfico instantaneamente
  // quando o cron (ou trigger manual) terminar uma rodada de bench.
  // Keepalive 20s evita que proxies (nginx/cloudflare) fechem a conexão
  // ociosa com ERR_INCOMPLETE_CHUNKED_ENCODING. Header X-Accel-Buffering
  // explícito desliga buffering em qualquer camada nginx no caminho.
  @Sse('bench-events')
  @Header('X-Accel-Buffering', 'no')
  @Header('Cache-Control', 'no-cache, no-transform')
  benchEventsStream(): Observable<NestMessageEvent> {
    const keepalive = interval(20_000).pipe(
      map(() => ({ data: JSON.stringify({ type: 'ping', ts: Date.now() }), type: 'ping' } as NestMessageEvent)),
    );
    return merge(BenchEventHub.stream(), keepalive);
  }

  // Cron/script chama isto após gravar bench_evolution.jsonl ou bench_snapshots.
  // Autenticado por X-Admin-Token (mesmo NCO_ADMIN_TOKEN do trigger).
  @Post('bench-events/emit')
  benchEventsEmit(
    @Headers() headers: Record<string, string>,
    @Body() body: any,
  ) {
    const adminToken = process.env.NCO_ADMIN_TOKEN || '';
    if (!adminToken) {
      throw new HttpException('Admin emit não configurado', HttpStatus.SERVICE_UNAVAILABLE);
    }
    const sent = (headers['x-admin-token'] as string) || '';
    if (sent !== adminToken) {
      throw new HttpException('Token admin inválido', HttpStatus.UNAUTHORIZED);
    }
    // Invalida caches pra próximo GET pegar dado fresco
    this._evCache = { ts: 0, data: null };
    this._benchCache = { ts: 0, data: null };
    BenchEventHub.emit({
      type: body?.type || 'bench-update',
      ts: new Date().toISOString(),
      ...(body || {}),
    });
    return { ok: true, emitted_at: new Date().toISOString() };
  }

  // Uso e plano do tenant logado — pra dashboard
  @Get('usage')
  async usage(@Req() req: Request) {
    const apiKey = (req.headers['x-api-key'] as string) || '';
    const tenant = await this.prisma.tenant.findUnique({
      where: { apiKey },
      select: { id: true, alias: true, companyName: true, plan: true, createdAt: true },
    });
    if (!tenant) {
      throw new HttpException('Tenant não encontrado', HttpStatus.UNAUTHORIZED);
    }
    const owner = await this.prisma.user.findFirst({
      where: { tenantId: tenant.id },
      select: { id: true, email: true, emailVerified: true },
    });
    const plan = (tenant.plan as string) || 'starter';
    const planSpec = NcoController.PLANS[plan as keyof typeof NcoController.PLANS] || NcoController.PLANS.starter;
    const now = new Date();
    const since1h = new Date(now.getTime() - 3600 * 1000);
    const recentNodes = await this.prisma.ncoCall.aggregate({
      where: { apiKey, ts: { gte: since1h }, statusCode: { lt: 400 } },
      _sum: { n: true },
      _count: true,
    });
    const bucket = this.getBucketSnapshot(apiKey, plan);
    return {
      tenant: { alias: tenant.alias, companyName: tenant.companyName },
      user: owner
        ? { email: owner.email, email_verified: owner.emailVerified }
        : null,
      plan,
      plan_spec: {
        max_n: planSpec.maxN,
        max_rollouts: planSpec.maxRollouts,
        nodes_per_sec: planSpec.nodesPerSec,
        burst_nodes: planSpec.burstNodes,
        price_brl: planSpec.priceBrl,
        label: planSpec.label,
      },
      bucket,
      usage_last_hour: {
        calls: recentNodes._count,
        nodes_total: recentNodes._sum.n ?? 0,
        nodes_per_sec_avg: ((recentNodes._sum.n ?? 0) / 3600).toFixed(2),
      },
      plans: NcoController.PLANS,
    };
  }

  @Get('metrics')
  async metrics(@Req() req: Request) {
    const apiKey = (req.headers['x-api-key'] as string) || '';
    const since = new Date(Date.now() - 30 * 24 * 3600 * 1000); // último mês
    const total = await this.prisma.ncoCall.count({ where: { apiKey, ts: { gte: since } } });
    const errors = await this.prisma.ncoCall.count({
      where: { apiKey, ts: { gte: since }, statusCode: { gte: 400 } },
    });
    const byEndpoint = await this.prisma.ncoCall.groupBy({
      by: ['endpoint'],
      where: { apiKey, ts: { gte: since } },
      _count: true,
      _avg: { timeMs: true, n: true },
    });
    return {
      window_days: 30,
      total_calls: total,
      error_rate: total > 0 ? errors / total : 0,
      by_endpoint: byEndpoint.map(e => ({
        endpoint: e.endpoint,
        count: e._count,
        avg_time_ms: e._avg.timeMs ? Math.round(e._avg.timeMs * 10) / 10 : null,
        avg_n: e._avg.n ? Math.round(e._avg.n) : null,
      })),
    };
  }

  // ---------- Proxy POST endpoints ----------
  private async proxyPost(endpoint: string, body: any, apiKey: string, apiAgent: string): Promise<any> {
    const t0 = Date.now();
    const n = body?.adjacency?.length || 0;
    let result: any = null;
    let statusCode = 200;
    let errorMsg: string | null = null;

    try {
      const r = await fetch(`${NCO_BASE}/api/nco/${endpoint}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(body),
        signal: AbortSignal.timeout(60000),
      });
      statusCode = r.status;
      if (!r.ok) {
        errorMsg = await r.text();
      } else {
        result = await r.json();
      }
    } catch (e: any) {
      statusCode = 500;
      errorMsg = e.message;
    }

    const elapsed = Date.now() - t0;
    // Persistir log (não-bloqueante)
    this.prisma.ncoCall.create({
      data: {
        apiKey, apiAgent, endpoint,
        n, modelName: result?.model || body?.model || null,
        rollouts: body?.rollouts || null,
        timeMs: result?.time_ms ?? elapsed,
        statusCode, errorMsg,
        result: result ? this.summarizeResult(endpoint, result) : null,
      },
    }).catch(e => this.logger.error(`log persist failed: ${e.message}`));

    if (statusCode >= 400) {
      throw new HttpException(errorMsg || 'NCO error', statusCode);
    }
    this.logger.log(`${endpoint} n=${n} t=${elapsed}ms key=${apiKey.slice(0, 8)}`);
    return result;
  }

  // Salvar só métrica + resumo (não a assignment completa de n=5000)
  private summarizeResult(endpoint: string, r: any): any {
    if (endpoint === 'maxcut') return { cut: r.cut };
    if (endpoint === 'coloring') return { conflicts: r.conflicts, n_edges: r.n_edges, k: r.k };
    if (endpoint === 'mis') return { size: r.size };
    if (endpoint === 'balance') return { agreements: r.agreements, disagreements: r.disagreements, balance_score: r.balance_score };
    return null;
  }

  // ---------- Plans + token bucket (rate por nodes/s) ----------
  // Source of truth: pra trocar limites é um lugar só.
  static readonly PLANS = {
    starter:    { maxN: 500,  maxRollouts: 32,  nodesPerSec: 100,    burstNodes: 500,     priceBrl: 0,   label: 'Starter (grátis)' },
    pro:        { maxN: 2000, maxRollouts: 64,  nodesPerSec: 2_000,  burstNodes: 10_000,  priceBrl: 299, label: 'Pro' },
    enterprise: { maxN: 10000, maxRollouts: 128, nodesPerSec: 20_000, burstNodes: 100_000, priceBrl: null, label: 'Enterprise' },
  } as const;

  // Token bucket em memória: { tokens, lastRefillMs }
  private static readonly _buckets = new Map<string, { tokens: number; lastRefillMs: number }>();

  private capsFor(agent: string, plan: string = 'starter'): { maxN: number; maxRollouts: number; nodesPerSec: number; burstNodes: number } {
    if (agent === 'demo') return { maxN: 50, maxRollouts: 16, nodesPerSec: 50, burstNodes: 50 }; // throttle por IP cobre o resto
    if (!agent.startsWith('tenant:')) {
      // interno (patricia/worker/admin) — sem rate
      return { maxN: 10000, maxRollouts: 128, nodesPerSec: Infinity, burstNodes: Infinity };
    }
    const p = NcoController.PLANS[(plan as keyof typeof NcoController.PLANS)] || NcoController.PLANS.starter;
    return { maxN: p.maxN, maxRollouts: p.maxRollouts, nodesPerSec: p.nodesPerSec, burstNodes: p.burstNodes };
  }

  private enforceCaps(body: any, agent: string, endpoint: string, plan: string): void {
    const n = body?.adjacency?.length || 0;
    const rollouts = body?.rollouts || 0;
    const caps = this.capsFor(agent, plan);
    const hardN = endpoint === 'mis' ? Math.min(caps.maxN, 2000) : caps.maxN;
    if (n > hardN) {
      throw new HttpException(
        `n=${n} excede o limite ${hardN} ${agent === 'demo' ? '(plano demo · crie conta grátis pra n maior)' : `(plano ${plan})`}.`,
        HttpStatus.PAYLOAD_TOO_LARGE,
      );
    }
    if (rollouts > caps.maxRollouts) {
      body.rollouts = caps.maxRollouts;
    }
  }

  // Token bucket: cada call consome `n` tokens; recarrega a `nodesPerSec` por segundo até `burstNodes`.
  private enforceRate(apiKey: string, plan: string, nNodes: number): void {
    const caps = this.capsFor(`tenant:_`, plan);
    if (!isFinite(caps.nodesPerSec)) return; // sem limit
    const now = Date.now();
    let b = NcoController._buckets.get(apiKey);
    if (!b) {
      b = { tokens: caps.burstNodes, lastRefillMs: now };
      NcoController._buckets.set(apiKey, b);
    }
    // refill
    const elapsedSec = (now - b.lastRefillMs) / 1000;
    b.tokens = Math.min(caps.burstNodes, b.tokens + elapsedSec * caps.nodesPerSec);
    b.lastRefillMs = now;
    // consume
    if (b.tokens >= nNodes) {
      b.tokens -= nNodes;
      return;
    }
    // sem tokens suficientes
    const needed = nNodes - b.tokens;
    const retrySec = Math.ceil(needed / caps.nodesPerSec);
    throw new HttpException(
      {
        message: `Rate limit do plano ${plan}: ${caps.nodesPerSec} nós/s · burst ${caps.burstNodes}. Tente em ${retrySec}s.`,
        plan,
        nodes_per_sec: caps.nodesPerSec,
        burst_nodes: caps.burstNodes,
        tokens_available: Math.floor(b.tokens),
        nodes_requested: nNodes,
        retry_after_seconds: retrySec,
        upgradeUrl: 'https://patriatechnology.com/nco?upgrade=1',
      },
      HttpStatus.TOO_MANY_REQUESTS,
    );
  }

  // Snapshot do bucket pra dashboard
  private getBucketSnapshot(apiKey: string, plan: string): { tokens: number; capacity: number; refill_per_sec: number; pct_full: number } {
    const caps = this.capsFor(`tenant:_`, plan);
    const cap = isFinite(caps.burstNodes) ? caps.burstNodes : 0;
    const rate = isFinite(caps.nodesPerSec) ? caps.nodesPerSec : 0;
    const b = NcoController._buckets.get(apiKey);
    if (!b) {
      return { tokens: cap, capacity: cap, refill_per_sec: rate, pct_full: 100 };
    }
    // refill virtual pra snapshot atualizado
    const now = Date.now();
    const tokens = Math.min(cap, b.tokens + ((now - b.lastRefillMs) / 1000) * rate);
    return {
      tokens: Math.floor(tokens),
      capacity: cap,
      refill_per_sec: rate,
      pct_full: cap > 0 ? Math.round((tokens / cap) * 100) : 0,
    };
  }

  // Carrega plan do tenant (via apiKey). Demo / interno passa "free" como neutro.
  private async resolvePlan(apiKey: string, agent: string): Promise<string> {
    if (!agent.startsWith('tenant:')) return 'free';
    const tenant = await this.prisma.tenant.findUnique({
      where: { apiKey },
      select: { plan: true },
    });
    return (tenant?.plan as string) || 'free';
  }

  @Post('maxcut')
  async maxcut(@Body() body: any, @Req() req: Request) {
    const apiKey = (req.headers['x-api-key'] as string) || '';
    const apiAgent = (req as any).apiAgent || 'unknown';
    const plan = await this.resolvePlan(apiKey, apiAgent);
    this.enforceCaps(body, apiAgent, 'maxcut', plan);
    if (apiAgent.startsWith('tenant:')) this.enforceRate(apiKey, plan, body?.adjacency?.length || 0);
    return this.proxyPost('maxcut', body, apiKey, apiAgent);
  }

  @Post('coloring')
  async coloring(@Body() body: any, @Req() req: Request) {
    const apiKey = (req.headers['x-api-key'] as string) || '';
    const apiAgent = (req as any).apiAgent || 'unknown';
    const plan = await this.resolvePlan(apiKey, apiAgent);
    this.enforceCaps(body, apiAgent, 'coloring', plan);
    if (apiAgent.startsWith('tenant:')) this.enforceRate(apiKey, plan, body?.adjacency?.length || 0);
    return this.proxyPost('coloring', body, apiKey, apiAgent);
  }

  @Post('mis')
  async mis(@Body() body: any, @Req() req: Request) {
    const apiKey = (req.headers['x-api-key'] as string) || '';
    const apiAgent = (req as any).apiAgent || 'unknown';
    const plan = await this.resolvePlan(apiKey, apiAgent);
    this.enforceCaps(body, apiAgent, 'mis', plan);
    if (apiAgent.startsWith('tenant:')) this.enforceRate(apiKey, plan, body?.adjacency?.length || 0);
    return this.proxyPost('mis', body, apiKey, apiAgent);
  }

  @Post('balance')
  async balance(@Body() body: any, @Req() req: Request) {
    const apiKey = (req.headers['x-api-key'] as string) || '';
    const apiAgent = (req as any).apiAgent || 'unknown';
    const plan = await this.resolvePlan(apiKey, apiAgent);
    this.enforceCaps(body, apiAgent, 'balance', plan);
    if (apiAgent.startsWith('tenant:')) this.enforceRate(apiKey, plan, body?.adjacency?.length || 0);
    return this.proxyPost('balance', body, apiKey, apiAgent);
  }
}
