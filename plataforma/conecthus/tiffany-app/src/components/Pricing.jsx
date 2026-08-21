import { Check, Star } from 'lucide-react'
import './Pricing.css'

const plans = [
  {
    name: 'Starter',
    price: 'R$ 497',
    period: '/mês',
    description: 'Ideal para pequenos negócios começando no digital.',
    features: [
      'Até 3 redes sociais',
      '30 publicações/mês com IA',
      'Geração de legendas e hashtags',
      'Agendamento automático',
      'Relatório mensal básico',
      'Suporte por e-mail',
    ],
    highlighted: false,
  },
  {
    name: 'Profissional',
    price: 'R$ 997',
    period: '/mês',
    description: 'Para empresas que querem escalar sua presença digital.',
    features: [
      'Até 6 redes sociais',
      '60 publicações/mês com IA',
      'Geração de conteúdo completo',
      'Tráfego pago (até R$ 5k/mês)',
      'Calendário editorial com IA',
      'Relatórios semanais detalhados',
      'Suporte prioritário via WhatsApp',
      'Análise de concorrentes',
    ],
    highlighted: true,
  },
  {
    name: 'Enterprise',
    price: 'Sob consulta',
    period: '',
    description: 'Solução completa para grandes operações.',
    features: [
      'Redes sociais ilimitadas',
      'Publicações ilimitadas com IA',
      'IA personalizada para sua marca',
      'Tráfego pago ilimitado',
      'Gestor de conta dedicado',
      'Relatórios em tempo real',
      'Integrações customizadas',
      'SLA garantido',
    ],
    highlighted: false,
  },
]

function Pricing() {
  return (
    <section id="planos" className="section">
      <div className="container">
        <div className="section-header">
          <span className="badge">Planos</span>
          <h2>Escolha o plano ideal <span className="highlight">para você</span></h2>
          <p>Opções flexíveis que acompanham o crescimento do seu negócio.</p>
        </div>
        <div className="pricing-grid">
          {plans.map((plan, index) => (
            <div
              key={index}
              className={`pricing-card ${plan.highlighted ? 'pricing-highlighted' : ''}`}
            >
              {plan.highlighted && (
                <div className="pricing-badge">
                  <Star size={14} /> Mais Popular
                </div>
              )}
              <h3>{plan.name}</h3>
              <div className="pricing-price">
                <span className="price-value">{plan.price}</span>
                <span className="price-period">{plan.period}</span>
              </div>
              <p className="pricing-description">{plan.description}</p>
              <ul className="pricing-features">
                {plan.features.map((feature, i) => (
                  <li key={i}>
                    <Check size={18} />
                    {feature}
                  </li>
                ))}
              </ul>
              <a
                href={plan.price === 'Sob consulta'
                  ? '#contato'
                  : (import.meta.env.VITE_APP_REGISTER_URL || 'https://app.patriatechnology.com/register')}
                className={`btn ${plan.highlighted ? 'btn-primary' : 'btn-secondary'} pricing-btn`}
                {...(plan.price !== 'Sob consulta' && { target: '_blank', rel: 'noopener noreferrer' })}
              >
                {plan.price === 'Sob consulta' ? 'Fale Conosco' : 'Começar Agora'}
              </a>
            </div>
          ))}
        </div>
      </div>
    </section>
  )
}

export default Pricing
