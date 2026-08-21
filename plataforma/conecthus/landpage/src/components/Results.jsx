import { TrendingUp, Users, Clock, Award } from 'lucide-react'
import './Results.css'

const stats = [
  { icon: TrendingUp, value: '300%', label: 'Aumento médio em engajamento' },
  { icon: Users, value: '500+', label: 'Empresas atendidas' },
  { icon: Clock, value: '40h', label: 'Economizadas por mês' },
  { icon: Award, value: '98%', label: 'Taxa de satisfação' },
]

const testimonials = [
  {
    name: 'Ana Silva',
    role: 'CEO, Boutique Digital',
    text: 'A Patria Technology revolucionou nossa presença digital. Em 3 meses, triplicamos nosso engajamento e dobramos as vendas vindas das redes sociais.',
  },
  {
    name: 'Carlos Mendes',
    role: 'Diretor de Marketing, TechBR',
    text: 'A automação com IA nos permitiu escalar nosso conteúdo sem perder qualidade. O ROI das campanhas de tráfego pago aumentou 250%.',
  },
  {
    name: 'Marina Costa',
    role: 'Fundadora, EcoStore',
    text: 'Economizo mais de 40 horas por mês com a automação. A IA gera conteúdos incríveis que realmente refletem a identidade da minha marca.',
  },
]

function Results() {
  return (
    <section id="resultados" className="section section-dark">
      <div className="container">
        <div className="section-header">
          <span className="badge">Resultados</span>
          <h2>Números que <span className="highlight">comprovam</span></h2>
          <p>Resultados reais de clientes que confiam na nossa tecnologia.</p>
        </div>

        <div className="stats-grid">
          {stats.map((stat, index) => (
            <div key={index} className="stat-card">
              <stat.icon size={28} className="stat-card-icon" />
              <div className="stat-value">{stat.value}</div>
              <div className="stat-label">{stat.label}</div>
            </div>
          ))}
        </div>

        <div className="testimonials-grid">
          {testimonials.map((t, index) => (
            <div key={index} className="testimonial-card">
              <p>"{t.text}"</p>
              <div className="testimonial-author">
                <div className="testimonial-avatar">
                  {t.name.charAt(0)}
                </div>
                <div>
                  <strong>{t.name}</strong>
                  <span>{t.role}</span>
                </div>
              </div>
            </div>
          ))}
        </div>
      </div>
    </section>
  )
}

export default Results
