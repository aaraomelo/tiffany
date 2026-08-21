import * as crypto from 'node:crypto';

const ALG = 'aes-256-gcm';
const KEY_LEN = 32;
const IV_LEN = 12;

function getKey(): Buffer {
  const secret = process.env.ASSISTANT_SECRET;
  if (!secret) {
    throw new Error('ASSISTANT_SECRET env not set');
  }
  return crypto.createHash('sha256').update(secret).digest().subarray(0, KEY_LEN);
}

export function encryptApiKey(plain: string): string {
  const iv = crypto.randomBytes(IV_LEN);
  const cipher = crypto.createCipheriv(ALG, getKey(), iv);
  const ct = Buffer.concat([cipher.update(plain, 'utf8'), cipher.final()]);
  const tag = cipher.getAuthTag();
  return [
    iv.toString('base64'),
    tag.toString('base64'),
    ct.toString('base64'),
  ].join('.');
}

export function decryptApiKey(stored: string): string | null {
  try {
    const [ivB64, tagB64, ctB64] = stored.split('.');
    if (!ivB64 || !tagB64 || !ctB64) return null;
    const iv = Buffer.from(ivB64, 'base64');
    const tag = Buffer.from(tagB64, 'base64');
    const ct = Buffer.from(ctB64, 'base64');
    const decipher = crypto.createDecipheriv(ALG, getKey(), iv);
    decipher.setAuthTag(tag);
    const plain = Buffer.concat([decipher.update(ct), decipher.final()]);
    return plain.toString('utf8');
  } catch {
    return null;
  }
}

/// Retorna 4 últimos chars da key pra mostrar no UI sem expor o segredo.
export function maskApiKey(stored: string | null | undefined): string | null {
  if (!stored) return null;
  const decrypted = decryptApiKey(stored);
  if (!decrypted) return null;
  if (decrypted.length <= 8) return '****';
  return `****${decrypted.slice(-4)}`;
}
