import { Injectable, Logger } from '@nestjs/common';
import { ConfigService } from '@nestjs/config';

interface EmbedResponse {
  vector: number[];
  vectors?: number[][];
  latency_ms: number;
  model: string;
  dim: number;
}

/// Cliente HTTP do daemon BGE-M3 (BAAI/bge-m3, 1024d, normalizado).
/// Default aponta para GEX44 (78.46.19.151:9302). Pode ser sobrescrito via env
/// para apontar para container local on-premise.
///
/// Fail-soft: se daemon estiver fora do ar, retorna null e os módulos
/// que usam embedding continuam funcionando (cai pra busca textual).
@Injectable()
export class EmbeddingService {
  private readonly logger = new Logger(EmbeddingService.name);
  private readonly baseUrl: string;
  private readonly apiKey: string;
  private readonly modelName: string;

  constructor(config: ConfigService) {
    this.baseUrl =
      config.get<string>('ERP_EMBEDDER_URL') ?? 'http://78.46.19.151:9302';
    this.apiKey =
      config.get<string>('ERP_EMBEDDER_KEY') ?? 'erp_dev_key_2026';
    this.modelName = 'BAAI/bge-m3';
  }

  get model(): string {
    return this.modelName;
  }

  /// Embedding único pra query/busca. Retorna null se daemon falhar.
  async embedQuery(text: string): Promise<number[] | null> {
    if (!text || text.trim().length < 2) return null;
    try {
      const res = await fetch(`${this.baseUrl}/embed-query`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'X-Embedder-Key': this.apiKey,
        },
        body: JSON.stringify({ text: text.slice(0, 4000), normalize: true }),
        signal: AbortSignal.timeout(8_000),
      });
      if (!res.ok) {
        this.logger.warn(`embed-query ${res.status}: ${await res.text()}`);
        return null;
      }
      const data = (await res.json()) as EmbedResponse;
      return data.vector ?? null;
    } catch (err) {
      this.logger.warn(`embed-query failed: ${(err as Error).message}`);
      return null;
    }
  }

  /// Embedding em batch (até 128 textos). Retorna null se daemon falhar.
  async embedBatch(texts: string[]): Promise<number[][] | null> {
    const cleaned = texts
      .filter((t) => t && t.trim().length >= 2)
      .map((t) => t.slice(0, 4000));
    if (cleaned.length === 0) return [];
    try {
      const res = await fetch(`${this.baseUrl}/embed`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'X-Embedder-Key': this.apiKey,
        },
        body: JSON.stringify({ texts: cleaned, normalize: true }),
        signal: AbortSignal.timeout(60_000),
      });
      if (!res.ok) {
        this.logger.warn(`embed batch ${res.status}: ${await res.text()}`);
        return null;
      }
      const data = (await res.json()) as EmbedResponse;
      return data.vectors ?? null;
    } catch (err) {
      this.logger.warn(`embed batch failed: ${(err as Error).message}`);
      return null;
    }
  }

  /// Health check (boot + manutenção).
  async health(): Promise<{ ok: boolean; model?: string; dim?: number }> {
    try {
      const res = await fetch(`${this.baseUrl}/health`, {
        signal: AbortSignal.timeout(3_000),
      });
      if (!res.ok) return { ok: false };
      const data = (await res.json()) as { ok: boolean; model: string; dim: number };
      return data;
    } catch {
      return { ok: false };
    }
  }

  /// Converte vetor JS para o literal de pgvector ("[v1,v2,...]").
  /// Usado dentro de $queryRawUnsafe pra evitar serialização ambígua.
  static toVectorLiteral(vec: number[]): string {
    return `[${vec.join(',')}]`;
  }
}
