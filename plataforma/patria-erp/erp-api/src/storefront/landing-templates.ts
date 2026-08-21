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
    case 'education':
      return build(name, {
        headline: name,
        subheadline: 'Ensino de qualidade para todas as idades',
        about: 'Turmas com profissionais qualificados, estrutura completa e foco no desenvolvimento de cada aluno.',
        ctaText: 'Faça sua matrícula',
        services: [
          { title: 'Turmas', description: 'Para todas as idades e níveis' },
          { title: 'Matrículas', description: 'Planos flexíveis' },
          { title: 'Acompanhamento', description: 'Evolução individual' },
        ],
      });
    case 'health':
      return build(name, {
        headline: name,
        subheadline: 'Cuidado e bem-estar pra você',
        about: 'Atendimento humano e profissional, com acompanhamento de saúde personalizado e estrutura completa.',
        ctaText: 'Agende sua avaliação',
        services: [
          { title: 'Avaliação', description: 'Anamnese e acompanhamento' },
          { title: 'Planos', description: 'Mensalidades flexíveis' },
          { title: 'Atendimento', description: 'Equipe qualificada' },
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
        subheadline: 'Diversão e bons momentos',
        about: 'O melhor do lazer e do entretenimento, com estrutura completa e a melhor experiência para você e sua família.',
        ctaText: 'Saiba mais',
        services: [
          { title: 'Programação', description: 'Agenda de atrações e atividades' },
          { title: 'Espaços', description: 'Ambientes para todos os públicos' },
          { title: 'Eventos', description: 'Festas e reservas' },
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
