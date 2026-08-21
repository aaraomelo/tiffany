import { Logger } from '@nestjs/common';

const BRIDGE_URL = process.env.BRIDGE_URL || 'http://host.docker.internal:9090';
const BRIDGE_SECRET = process.env.BRIDGE_SECRET || 'wk_infer_patria_2026';

const logger = new Logger('BridgeClient');

export class BridgeError extends Error {
  constructor(
    public readonly endpoint: string,
    public readonly statusCode: number,
    message: string,
  ) {
    super(`[Bridge ${endpoint}] ${message}`);
    this.name = 'BridgeError';
  }
}

export async function bridgeCall<T = any>(
  endpoint: string,
  body: Record<string, any>,
  timeoutMs = 30_000,
): Promise<T> {
  const url = `${BRIDGE_URL}${endpoint}`;

  try {
    const res = await fetch(url, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'X-Bridge-Key': BRIDGE_SECRET,
      },
      body: JSON.stringify(body),
      signal: AbortSignal.timeout(timeoutMs),
    });

    const data = await res.json().catch(() => ({ error: `HTTP ${res.status} (no JSON body)` }));

    if (!res.ok) {
      const msg = data.error || `HTTP ${res.status}`;
      logger.error(`${endpoint} failed: ${msg}`);
      throw new BridgeError(endpoint, res.status, msg);
    }

    return data as T;
  } catch (err) {
    if (err instanceof BridgeError) throw err;

    // Network/timeout errors
    const msg = err.message || 'Unknown error';
    if (msg.includes('fetch failed') || msg.includes('ECONNREFUSED')) {
      logger.error(`Bridge unreachable at ${BRIDGE_URL} — is claude-bridge running?`);
      throw new BridgeError(endpoint, 0, `Bridge unreachable (${BRIDGE_URL}): ${msg}`);
    }
    if (msg.includes('TimeoutError') || msg.includes('aborted')) {
      logger.error(`${endpoint} timed out after ${timeoutMs}ms`);
      throw new BridgeError(endpoint, 0, `Timeout after ${Math.round(timeoutMs / 1000)}s`);
    }

    throw new BridgeError(endpoint, 0, msg);
  }
}
