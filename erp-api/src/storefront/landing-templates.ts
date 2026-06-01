// Conteúdo inicial da landing page do cliente, por segmento. Usado no cadastro
// self-service: o cliente já sai com uma página pronta pra editar.

export interface LandingTemplate {
  headline: string;
  subheadline: string;
  about: string;
  ctaText: string;
  services: { title: string; description?: string }[];
}

function build(name: string, t: Omit<LandingTemplate, 'headline'> & { headline?: string }): LandingTemplate {
  return { headline: t.headline ?? name, ...t };
}

export function landingTemplateFor(segment: string, name: string): LandingTemplate {
  switch (segment) {
    case 'football_school':
      return build(name, {
        headline: name,
        subheadline: 'Formando atletas e cidadãos',
        about: 'Treinos para todas as idades com profissionais qualificados, estrutura completa e foco no desenvolvimento técnico e humano.',
        ctaText: 'Agende uma aula experimental',
        services: [
          { title: 'Categorias de base', description: 'Sub-9 ao Sub-17' },
          { title: 'Escolinha infantil', description: 'A partir de 4 anos' },
          { title: 'Avaliação física', description: 'Acompanhamento individual' },
        ],
      });
    case 'parts_service':
      return build(name, {
        subheadline: 'Peças e serviços com qualidade e garantia',
        about: 'Atendimento especializado, mão de obra qualificada e peças de procedência. Orçamento sem compromisso.',
        ctaText: 'Solicitar orçamento',
        services: [
          { title: 'Manutenção', description: 'Preventiva e corretiva' },
          { title: 'Venda de peças', description: 'Originais e paralelas' },
          { title: 'Garantia', description: 'Serviço com termo de garantia' },
        ],
      });
    case 'construction':
      return build(name, {
        subheadline: 'Tudo para sua obra num só lugar',
        about: 'Materiais de construção com os melhores preços, entrega rápida e atendimento de quem entende do assunto.',
        ctaText: 'Fale conosco',
        services: [
          { title: 'Material básico', description: 'Cimento, areia, brita' },
          { title: 'Acabamento', description: 'Pisos, tintas, louças' },
          { title: 'Entrega', description: 'Direto na sua obra' },
        ],
      });
    case 'food':
      return build(name, {
        subheadline: 'Sabor que conquista',
        about: 'Pratos preparados com ingredientes frescos e muito carinho. Venha nos visitar ou peça pelo delivery.',
        ctaText: 'Ver cardápio',
        services: [
          { title: 'Salão', description: 'Ambiente aconchegante' },
          { title: 'Delivery', description: 'Entrega na sua casa' },
          { title: 'Eventos', description: 'Encomendas e festas' },
        ],
      });
    case 'event':
      return build(name, {
        subheadline: 'Momentos inesquecíveis',
        about: 'Produção de eventos com estrutura completa, line-up de qualidade e a melhor experiência para o seu público.',
        ctaText: 'Comprar ingresso',
        services: [
          { title: 'Shows', description: 'Agenda de atrações' },
          { title: 'Camarote', description: 'Experiência premium' },
          { title: 'Eventos privados', description: 'Aniversários e corporativo' },
        ],
      });
    default: // commerce e genéricos
      return build(name, {
        subheadline: 'Qualidade e bom atendimento',
        about: `Bem-vindo à ${name}. Conheça nossos produtos e venha nos visitar.`,
        ctaText: 'Fale conosco',
        services: [
          { title: 'Variedade', description: 'Produtos para todos os gostos' },
          { title: 'Atendimento', description: 'Equipe pronta pra ajudar' },
          { title: 'Confiança', description: 'Qualidade garantida' },
        ],
      });
  }
}
