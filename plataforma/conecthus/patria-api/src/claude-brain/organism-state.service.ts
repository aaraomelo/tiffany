import { Injectable, Logger } from '@nestjs/common';
import { execSync } from 'child_process';

const GEX44_HOST = process.env.GEX44_HOST_FOR_SCP || 'claude@78.46.19.151';
const GEX44_SSH_KEY = process.env.GEX44_SSH_KEY || '/root/.ssh/id_ed25519_gex44';
const STOKES_PATH = '/home/claude/bus/stokes.json';
const CACHE_MS = 60_000;

@Injectable()
export class OrganismStateService {
  private readonly logger = new Logger('OrganismState');
  private _cache: { ts: number; data: any } | null = null;

  /** Snapshot resumido do multiverso. Cache 60s pra evitar SSH em rajada. */
  async getState(forceFresh = false): Promise<any> {
    const now = Date.now();
    if (!forceFresh && this._cache && now - this._cache.ts < CACHE_MS) {
      return { ...this._cache.data, cached: true, cache_age_ms: now - this._cache.ts };
    }

    let stokes: any;
    try {
      const raw = execSync(
        `ssh -i ${GEX44_SSH_KEY} -o StrictHostKeyChecking=accept-new ` +
        `-o ConnectTimeout=10 ${GEX44_HOST} 'cat ${STOKES_PATH} 2>/dev/null'`,
        { encoding: 'utf-8', timeout: 20_000, maxBuffer: 16 * 1024 * 1024 },
      );
      // Python emite NaN/Infinity em json.dump; JSON spec não permite — substitui por null
      const sanitized = raw
        .replace(/\bNaN\b/g, 'null')
        .replace(/-Infinity\b/g, 'null')
        .replace(/\bInfinity\b/g, 'null');
      stokes = JSON.parse(sanitized);
    } catch (e: any) {
      return {
        ok: false,
        error: e.message?.slice(0, 200) || 'ssh failed',
        ts: new Date().toISOString(),
      };
    }

    // Procs críticos (best-effort, falha silenciosa)
    let n_procs: number | null = null;
    try {
      const out = execSync(
        `ssh -i ${GEX44_SSH_KEY} -o ConnectTimeout=8 ${GEX44_HOST} ` +
        `'ps aux | grep -E "python (stokes_daemon|meta_veia|coracao_central)\\.py" | grep -v grep | wc -l'`,
        { encoding: 'utf-8', timeout: 12_000 },
      );
      n_procs = parseInt(out.trim(), 10);
    } catch {}

    // Última linha do meta_veia.log com WARN/ERROR
    let recent_warn: string | null = null;
    try {
      const out = execSync(
        `ssh -i ${GEX44_SSH_KEY} -o ConnectTimeout=8 ${GEX44_HOST} ` +
        `'tail -50 /home/claude/bus/meta_veia.log 2>/dev/null | grep -E "WARN|ERROR" | tail -1'`,
        { encoding: 'utf-8', timeout: 12_000 },
      );
      recent_warn = out.trim().slice(0, 400) || null;
    } catch {}

    const universes: Record<string, any> = {};
    const universesData = stokes.universes || {};
    for (const [uid, info] of Object.entries(universesData) as [string, any][]) {
      universes[uid] = {
        alpha: info.median_alpha_long,
        ac_dc: info.median_ac_dc,
        cells: info.n_cells,
      };
    }

    const data = {
      ok: true,
      ts: new Date().toISOString(),
      stokes_ts: stokes.ts,
      stokes_age_seconds: stokes.ts ? Math.round(now / 1000 - stokes.ts) : null,
      universes_count: Object.keys(universes).length,
      universes,
      critical_procs_count: n_procs,
      critical_procs_expected: 3,
      recent_warn_or_error: recent_warn,
      cached: false,
    };
    this._cache = { ts: now, data };
    return data;
  }

  /** Resumo curto pra Patrícia LLM consumir como tool result. */
  async getSummary(): Promise<string> {
    const s = await this.getState();
    if (!s.ok) return `Multiverso INDISPONÍVEL: ${s.error}`;
    const lines: string[] = [];
    const ageMin = s.stokes_age_seconds != null ? Math.round(s.stokes_age_seconds / 60) : '?';
    lines.push(`📡 Multiverso (snapshot ${ageMin}min atrás):`);
    lines.push(`${s.universes_count} universos visíveis. Procs críticos: ${s.critical_procs_count}/${s.critical_procs_expected}.`);
    const us: string[] = [];
    for (const [uid, info] of Object.entries(s.universes) as [string, any][]) {
      const a = info.alpha != null ? info.alpha.toFixed(3) : '?';
      const ac = info.ac_dc != null ? info.ac_dc.toExponential(2) : '?';
      us.push(`${uid}: α=${a} ac_dc=${ac} cells=${info.cells}`);
    }
    lines.push(us.join(' | '));
    if (s.recent_warn_or_error) lines.push(`⚠ ${s.recent_warn_or_error}`);
    return lines.join('\n');
  }
}
