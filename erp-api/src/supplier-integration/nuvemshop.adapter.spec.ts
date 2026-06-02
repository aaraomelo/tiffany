import {
  decodeHtmlEntities,
  extractGallery,
  extractLocs,
  mapNuvemshopVariant,
  parseProductPage,
} from './nuvemshop.adapter';

describe('NuvemshopAdapter — parsers puros', () => {
  describe('decodeHtmlEntities', () => {
    it('decodifica aspas, e-comercial e numéricas (na ordem certa)', () => {
      expect(decodeHtmlEntities('&quot;a&quot;')).toBe('"a"');
      expect(decodeHtmlEntities('Preto &amp; Branco')).toBe('Preto & Branco');
      expect(decodeHtmlEntities('M&#39;e')).toBe("M'e");
      // &amp; decodificado por último: não vira aspas em &amp;quot;
      expect(decodeHtmlEntities('&amp;quot;')).toBe('&quot;');
    });
  });

  describe('extractLocs', () => {
    it('extrai todas as <loc> do sitemap', () => {
      const xml = `<urlset>
        <url><loc>https://x.com/produtos/a/</loc></url>
        <url><loc> https://x.com/produtos/b/ </loc></url>
        <url><loc>https://x.com/contato/</loc></url>
      </urlset>`;
      expect(extractLocs(xml)).toEqual([
        'https://x.com/produtos/a/',
        'https://x.com/produtos/b/',
        'https://x.com/contato/',
      ]);
    });
  });

  describe('mapNuvemshopVariant', () => {
    it('mapeia os campos comprovados do data-variants real', () => {
      const raw = {
        product_id: 280861017,
        id: 731001,
        sku: '731-1',
        price_number: 41.17,
        price_with_payment_discount_short: 'R$34,99',
        stock: 11,
        available: true,
        option0: 'Preto',
        option1: 'P veste 40',
        option2: null,
        image_url: 'https://cdn/x.jpg',
      };
      const v = mapNuvemshopVariant(raw);
      expect(v).toMatchObject({
        externalProductId: '280861017',
        externalVariantId: '731001',
        sku: '731-1',
        price: 41.17,
        pixPrice: 'R$34,99',
        stock: 11,
        available: true,
        option0: 'Preto',
        option1: 'P veste 40',
        imageUrl: 'https://cdn/x.jpg',
      });
    });

    it('retorna null sem id de variação', () => {
      expect(mapNuvemshopVariant({ sku: 'x' })).toBeNull();
      expect(mapNuvemshopVariant(null)).toBeNull();
    });

    it('available default true quando ausente', () => {
      expect(mapNuvemshopVariant({ id: 1 })?.available).toBe(true);
      expect(mapNuvemshopVariant({ id: 1, available: false })?.available).toBe(false);
    });
  });

  describe('extractGallery', () => {
    it('deduplica resoluções pelo arquivo-base e prefere a maior', () => {
      const html = `
        <img src="//acdn-us.mitiendanube.com/stores/001/684/261/products/abc-5ab14d-240-0.webp">
        <img src="//acdn-us.mitiendanube.com/stores/001/684/261/products/abc-5ab14d-1024-1024.webp">
        <img src="//acdn-us.mitiendanube.com/stores/001/684/261/products/def-99887-480-0.webp">
      `;
      const g = extractGallery(html);
      expect(g).toHaveLength(2); // 2 fotos-base distintas
      expect(g.every((u) => u.startsWith('https://'))).toBe(true);
      // base abc → escolhe a 1024, não a 240
      expect(g.some((u) => u.includes('abc-5ab14d-1024-1024'))).toBe(true);
      expect(g.some((u) => u.includes('abc-5ab14d-240-0'))).toBe(false);
    });

    it('retorna vazio sem imagens de produto', () => {
      expect(extractGallery('<html>nada</html>')).toEqual([]);
    });
  });

  describe('parseProductPage', () => {
    // Fixture com data-variants escapado, como vem do tema Nuvemshop.
    const variantsJson = JSON.stringify([
      {
        product_id: 280861017,
        id: 731001,
        price_number: 41.17,
        price_with_payment_discount_short: 'R$34,99',
        stock: 1,
        sku: '731-1',
        available: true,
        option0: 'Preto',
        option1: 'P veste 40',
      },
      {
        product_id: 280861017,
        id: 731005,
        price_number: 41.17,
        price_with_payment_discount_short: 'R$34,99',
        stock: 0,
        sku: '731-5',
        available: false,
        option0: 'Preto',
        option1: 'M veste 42',
      },
    ]);
    const escaped = variantsJson
      .replace(/&/g, '&amp;')
      .replace(/"/g, '&quot;');
    const html = `<html><head>
      <meta property="og:title" content="Conjunto Starjane Secret" />
      </head><body>
      <div id="single-product-container" class="js-product-container" data-variants="${escaped}">
      </div></body></html>`;

    it('extrai nome e todas as variações', () => {
      const p = parseProductPage(html, 'https://x.com/produtos/conjunto-starjane-secret/');
      expect(p.name).toBe('Conjunto Starjane Secret');
      expect(p.variants).toHaveLength(2);
      expect(p.variants[0].sku).toBe('731-1');
      expect(p.variants[0].price).toBe(41.17);
      expect(p.variants[1].available).toBe(false);
      expect(p.variants[1].stock).toBe(0);
    });

    it('retorna vazio (sem quebrar) quando não há container', () => {
      const p = parseProductPage('<html><title>X</title></html>', 'u');
      expect(p.variants).toEqual([]);
      expect(p.name).toBe('X');
    });

    it('tolera data-variants inválido', () => {
      const bad = `<div id="single-product-container" data-variants="&quot;not-json"></div>`;
      expect(parseProductPage(bad, 'u').variants).toEqual([]);
    });
  });
});
