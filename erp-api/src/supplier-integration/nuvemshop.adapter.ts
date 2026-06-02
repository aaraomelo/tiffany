import { Injectable, Logger } from '@nestjs/common';
import { ConfigService } from '@nestjs/config';

/// Uma variação de produto, do JSON `data-variants` embutido na página Nuvemshop.
export interface NuvemshopVariant {
  externalProductId: string;
  externalVariantId: string;
  sku: string | null;
  gtin: string | null;
  price: number;
  pixPrice: string | null;
  stock: number | null;
  available: boolean;
  option0: string | null; // ex: cor
  option1: string | null; // ex: tamanho
  option2: string | null;
  imageUrl: string | null;
}

export interface NuvemshopProduct {
  url: string;
  name: string;
  variants: NuvemshopVariant[];
}

export interface NuvemshopLogin {
  cookie: string;
  expiresAt: Date;
}

export interface OrderPreviewItem {
  externalVariantId: string;
  qty: number;
  unitPrice: number;
  lineTotal: number;
}

export interface OrderPreview {
  items: OrderPreviewItem[];
  total: number;
  cartUrl: string;
  submitted: boolean;
}

/// Erro sinalizando que o cookie de sessão expirou (redirect → /account/login/).
export class SessionExpiredError extends Error {
  constructor() {
    super('Sessão do fornecedor expirou — re-login necessário');
    this.name = 'SessionExpiredError';
  }
}

const UA =
  'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/124.0 Safari/537.36';
const HTTP_TIMEOUT_MS = 20_000;

/// Adapter de loja Nuvemshop (login do cliente + scraping de catálogo + carrinho).
///
/// - Catálogo é PÚBLICO: `sitemap.xml` lista as URLs de produto; cada página
///   server-rendered traz o JSON `data-variants` com todas as variações.
/// - Login: POST /account/login/ (email+password, sem CSRF) → 302, sessão por
///   cookie `store_login_session`. Só necessário pra pedido / histórico.
/// - Carrinho: POST /comprar/ (add_to_cart=<variantId>, quantity).
///
/// Modo stub (default): quando SUPPLIER_INTEGRATION_LIVE != 'on', nenhuma rede
/// externa é tocada — retorna dados sintéticos pra dev local rodar end-to-end.
/// Submissão real de pedido (submit=true) é bloqueada mesmo em modo live (Fase B2).
@Injectable()
export class NuvemshopAdapter {
  private readonly logger = new Logger(NuvemshopAdapter.name);
  private readonly live: boolean;

  constructor(config: ConfigService) {
    this.live = config.get<string>('SUPPLIER_INTEGRATION_LIVE') === 'on';
    if (!this.live) {
      this.logger.warn(
        'SUPPLIER_INTEGRATION_LIVE != on — adapter em modo STUB (sem rede externa)',
      );
    }
  }

  get isStub() {
    return !this.live;
  }

  // ---------------------------------------------------------------------------
  // Login
  // ---------------------------------------------------------------------------

  async login(
    baseUrl: string,
    email: string,
    password: string,
  ): Promise<NuvemshopLogin> {
    if (this.isStub) {
      return {
        cookie: `store_login_session=STUB_${Buffer.from(email).toString('base64').slice(0, 12)}`,
        expiresAt: new Date(Date.now() + 7 * 24 * 3600 * 1000),
      };
    }

    const root = this.normalizeBase(baseUrl);
    // 1) GET da página de login pra obter os cookies iniciais.
    const pre = await this.fetchRaw(`${root}/account/login/`, { method: 'GET' });
    const preCookie = this.collectCookies(pre, '');

    // 2) POST das credenciais (form-urlencoded, sem CSRF). Espera 302.
    const body = new URLSearchParams();
    body.set('email', email);
    body.set('password', password);
    const res = await this.fetchRaw(`${root}/account/login/`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/x-www-form-urlencoded',
        Cookie: preCookie,
        Referer: `${root}/account/login/`,
      },
      body: body.toString(),
      redirect: 'manual',
    });

    const cookie = this.collectCookies(res, preCookie);
    const location = res.headers.get('location') ?? '';
    const ok = res.status === 302 && !/\/account\/login/.test(location);
    if (!ok) {
      throw new Error(
        `Login falhou (status ${res.status}). Verifique email/senha.`,
      );
    }
    return { cookie, expiresAt: new Date(Date.now() + 24 * 3600 * 1000) };
  }

  // ---------------------------------------------------------------------------
  // Catálogo (público)
  // ---------------------------------------------------------------------------

  /// Lista todas as URLs de produto a partir do sitemap. Trata sitemap index
  /// (um nível de aninhamento).
  async listProductUrls(baseUrl: string): Promise<string[]> {
    if (this.isStub) {
      const root = this.normalizeBase(baseUrl);
      return [
        `${root}/produtos/produto-demo-1/`,
        `${root}/produtos/produto-demo-2/`,
      ];
    }

    const root = this.normalizeBase(baseUrl);
    const xml = await this.fetchText(`${root}/sitemap.xml`);
    const locs = extractLocs(xml);

    const productUrls: string[] = [];
    const nested: string[] = [];
    for (const loc of locs) {
      if (/\/produtos\//.test(loc)) productUrls.push(loc);
      else if (/\.xml(\.gz)?$/.test(loc) && loc !== `${root}/sitemap.xml`)
        nested.push(loc);
    }

    // Um nível de sitemaps aninhados (índice → urlsets).
    for (const sub of nested) {
      try {
        const subXml = await this.fetchText(sub);
        for (const loc of extractLocs(subXml)) {
          if (/\/produtos\//.test(loc)) productUrls.push(loc);
        }
      } catch (err) {
        this.logger.warn(
          `sitemap aninhado ${sub} falhou: ${(err as Error).message}`,
        );
      }
    }

    // Dedup + remove a própria página de listagem /produtos/.
    return [...new Set(productUrls)].filter((u) => !/\/produtos\/?$/.test(u));
  }

  /// Baixa uma página de produto e extrai nome + variações do `data-variants`.
  async fetchProduct(baseUrl: string, url: string): Promise<NuvemshopProduct> {
    if (this.isStub) {
      const id = url.replace(/\/+$/, '').split('/').pop() ?? 'demo';
      return {
        url,
        name: `Produto Demo (${id})`,
        variants: [
          {
            externalProductId: `stub-${id}`,
            externalVariantId: `stub-${id}-P`,
            sku: `STUB-${id}-P`,
            gtin: null,
            price: 41.17,
            pixPrice: 'R$34,99',
            stock: 10,
            available: true,
            option0: 'Preto',
            option1: 'P',
            option2: null,
            imageUrl: null,
          },
        ],
      };
    }

    const html = await this.fetchText(url);
    return parseProductPage(html, url);
  }

  // ---------------------------------------------------------------------------
  // Carrinho / pedido
  // ---------------------------------------------------------------------------

  async addToCart(
    baseUrl: string,
    cookie: string,
    variantId: string,
    qty: number,
  ): Promise<void> {
    if (this.isStub) return;
    const root = this.normalizeBase(baseUrl);
    const body = new URLSearchParams();
    body.set('add_to_cart', variantId);
    body.set('quantity', String(qty));
    const res = await this.fetchRaw(`${root}/comprar/`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/x-www-form-urlencoded',
        Cookie: cookie,
      },
      body: body.toString(),
      redirect: 'manual',
    });
    if (this.isLoginRedirect(res)) throw new SessionExpiredError();
    if (res.status >= 400) {
      throw new Error(`addToCart falhou (status ${res.status})`);
    }
  }

  /// Monta o carrinho e retorna um preview. NUNCA finaliza o pedido por padrão.
  /// `submit=true` é Fase B2 (checkout reverse-engineered) e está bloqueado.
  async placeOrder(
    baseUrl: string,
    cookie: string,
    items: Array<{ externalVariantId: string; qty: number; unitPrice: number }>,
    opts: { submit?: boolean } = {},
  ): Promise<OrderPreview> {
    const root = this.normalizeBase(baseUrl);
    const previewItems: OrderPreviewItem[] = items.map((it) => ({
      externalVariantId: it.externalVariantId,
      qty: it.qty,
      unitPrice: it.unitPrice,
      lineTotal: round2(it.unitPrice * it.qty),
    }));
    const total = round2(previewItems.reduce((s, i) => s + i.lineTotal, 0));

    if (opts.submit) {
      // Fase B2: finalização real do checkout ainda não implementada.
      throw new Error(
        'Submissão real de pedido ainda não implementada (Fase B2). Use dry-run.',
      );
    }

    if (!this.isStub) {
      // Dry-run real: monta o carrinho (sem finalizar), só pra validar disponibilidade.
      for (const it of items) {
        await this.addToCart(root, cookie, it.externalVariantId, it.qty);
      }
    }

    return {
      items: previewItems,
      total,
      cartUrl: `${root}/comprar/`,
      submitted: false,
    };
  }

  // ---------------------------------------------------------------------------
  // Helpers de concorrência
  // ---------------------------------------------------------------------------

  /// Roda `fn` sobre `items` com concorrência limitada + pausa entre lotes
  /// (polidez/rate-limit no sweep do catálogo).
  async mapWithConcurrency<T, R>(
    items: T[],
    limit: number,
    fn: (item: T, index: number) => Promise<R>,
    delayMs = 150,
  ): Promise<R[]> {
    const results: R[] = new Array(items.length);
    let cursor = 0;
    const worker = async () => {
      while (cursor < items.length) {
        const i = cursor++;
        results[i] = await fn(items[i], i);
        if (delayMs > 0) await sleep(delayMs);
      }
    };
    const pool = Array.from({ length: Math.min(limit, items.length) }, () =>
      worker(),
    );
    await Promise.all(pool);
    return results;
  }

  // ---------------------------------------------------------------------------
  // Internos
  // ---------------------------------------------------------------------------

  private normalizeBase(baseUrl: string): string {
    return baseUrl.replace(/\/+$/, '');
  }

  private async fetchRaw(
    url: string,
    init: RequestInit,
  ): Promise<Response> {
    return fetch(url, {
      ...init,
      headers: { 'User-Agent': UA, ...(init.headers ?? {}) },
      signal: AbortSignal.timeout(HTTP_TIMEOUT_MS),
    });
  }

  private async fetchText(url: string): Promise<string> {
    const res = await this.fetchRaw(url, { method: 'GET' });
    if (this.isLoginRedirect(res)) throw new SessionExpiredError();
    if (!res.ok) throw new Error(`GET ${url} → ${res.status}`);
    return res.text();
  }

  private isLoginRedirect(res: Response): boolean {
    const loc = res.headers.get('location') ?? '';
    return (
      (res.status === 301 || res.status === 302) && /\/account\/login/.test(loc)
    );
  }

  /// Junta os Set-Cookie da resposta com os cookies já existentes em "name=value; ...".
  private collectCookies(res: Response, existing: string): string {
    const jar = new Map<string, string>();
    for (const pair of existing.split(';')) {
      const [k, ...rest] = pair.trim().split('=');
      if (k) jar.set(k, rest.join('='));
    }
    const setCookies = res.headers.getSetCookie?.() ?? [];
    for (const sc of setCookies) {
      const first = sc.split(';')[0];
      const [k, ...rest] = first.split('=');
      if (k) jar.set(k.trim(), rest.join('='));
    }
    return [...jar.entries()].map(([k, v]) => `${k}=${v}`).join('; ');
  }
}

// =============================================================================
// Helpers livres de estado
// =============================================================================

/// Extrai todas as URLs <loc> de um sitemap XML.
export function extractLocs(xml: string): string[] {
  const out: string[] = [];
  const re = /<loc>\s*([^<]+?)\s*<\/loc>/g;
  let m: RegExpExecArray | null;
  while ((m = re.exec(xml))) out.push(m[1].trim());
  return out;
}

/// Nome do produto a partir do HTML (og:title → h1 → title).
export function extractName(html: string): string {
  const og = /<meta\s+property="og:title"\s+content="([^"]+)"/i.exec(html);
  if (og) return decodeHtmlEntities(og[1]).trim();
  const h1 = /<h1[^>]*>([^<]+)<\/h1>/i.exec(html);
  if (h1) return decodeHtmlEntities(h1[1]).trim();
  const t = /<title>([^<]+)<\/title>/i.exec(html);
  return t ? decodeHtmlEntities(t[1]).trim() : 'Produto';
}

/// Mapeia uma entrada bruta do data-variants pro nosso tipo.
export function mapNuvemshopVariant(v: unknown): NuvemshopVariant | null {
  if (!v || typeof v !== 'object') return null;
  const o = v as Record<string, unknown>;
  const productId = o.product_id ?? o.productId;
  const variantId = o.id ?? o.variant_id;
  if (variantId == null) return null;
  return {
    externalProductId: String(productId ?? variantId),
    externalVariantId: String(variantId),
    sku: o.sku != null ? String(o.sku) : null,
    gtin: typeof o.barcode === 'string' ? o.barcode : null,
    price: toNumber(o.price_number ?? o.price),
    pixPrice:
      typeof o.price_with_payment_discount_short === 'string'
        ? o.price_with_payment_discount_short
        : null,
    stock: o.stock == null ? null : Number(o.stock),
    available: o.available !== false,
    option0: o.option0 != null ? String(o.option0) : null,
    option1: o.option1 != null ? String(o.option1) : null,
    option2: o.option2 != null ? String(o.option2) : null,
    imageUrl:
      typeof o.image_url === 'string'
        ? o.image_url
        : typeof o.image === 'string'
          ? o.image
          : null,
  };
}

/// Parser puro de uma página de produto Nuvemshop: nome + variações do
/// atributo `data-variants` no container principal do produto.
export function parseProductPage(html: string, url: string): NuvemshopProduct {
  const name = extractName(html);
  const idx = html.indexOf('single-product-container');
  if (idx < 0) return { url, name, variants: [] };
  const m = /data-variants="([^"]*)"/.exec(html.slice(idx));
  if (!m) return { url, name, variants: [] };
  let raw: unknown;
  try {
    raw = JSON.parse(decodeHtmlEntities(m[1]));
  } catch {
    return { url, name, variants: [] };
  }
  const arr = Array.isArray(raw) ? raw : [];
  const variants = arr
    .map((v) => mapNuvemshopVariant(v))
    .filter((v): v is NuvemshopVariant => v !== null);
  return { url, name, variants };
}

/// Decodifica entidades HTML básicas presentes no atributo data-variants.
export function decodeHtmlEntities(s: string): string {
  return s
    .replace(/&quot;/g, '"')
    .replace(/&#34;/g, '"')
    .replace(/&#39;/g, "'")
    .replace(/&apos;/g, "'")
    .replace(/&lt;/g, '<')
    .replace(/&gt;/g, '>')
    .replace(/&#(\d+);/g, (_, n: string) => String.fromCharCode(Number(n)))
    .replace(/&amp;/g, '&');
}

function toNumber(v: unknown): number {
  if (typeof v === 'number') return v;
  if (typeof v === 'string') {
    const n = Number(v.replace(/[^\d.,-]/g, '').replace(',', '.'));
    return Number.isFinite(n) ? n : 0;
  }
  return 0;
}

function round2(n: number): number {
  return Math.round(n * 100) / 100;
}

function sleep(ms: number): Promise<void> {
  return new Promise((r) => setTimeout(r, ms));
}
