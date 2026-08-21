import { Bot, PenTool, Target, Calendar, BarChart3, Megaphone } from 'lucide-react'
import './Services.css'

const services = [
  {
    icon: Bot,
    title: 'Automação de Redes Sociais',
    description: 'Publicações automáticas no Instagram, Facebook, LinkedIn e X com horários otimizados por IA para máximo alcance.',
  },
  {
    icon: PenTool,
    title: 'Geração de Conteúdo com IA',
    description: 'Textos, legendas, hashtags e roteiros criados automaticamente pela IA, alinhados com a voz da sua marca.',
  },
  {
    icon: Target,
    title: 'Tráfego Pago Inteligente',
    description: 'Campanhas de Meta Ads e Google Ads otimizadas em tempo real com inteligência artificial para maximizar seu ROI.',
  },
  {
    icon: Calendar,
    title: 'Planejamento Editorial',
    description: 'Calendário de conteúdo gerado automaticamente com base em tendências, datas comemorativas e comportamento do público.',
  },
  {
    icon: BarChart3,
    title: 'Análise de Desempenho',
    description: 'Dashboards inteligentes com insights acionáveis sobre suas campanhas, engajamento e crescimento.',
  },
  {
    icon: Megaphone,
    title: 'Gestão de Campanhas',
    description: 'Criação, monitoramento e otimização completa de campanhas publicitárias com acompanhamento em tempo real.',
  },
]

function Services() {
  return (
    <section id="servicos" className="section">
      <div className="container">
        <div className="section-header">
          <span className="badge">Serviços</span>
          <h2>Soluções completas em <span className="highlight">automação com IA</span></h2>
          <p>
            Ferramentas poderosas que transformam a maneira como você gerencia suas redes sociais e campanhas digitais.
          </p>
        </div>
        <div className="services-grid">
          {services.map((service, index) => (
            <div key={index} className="service-card">
              <div className="service-icon">
                <service.icon size={28} />
              </div>
              <h3>{service.title}</h3>
              <p>{service.description}</p>
            </div>
          ))}
        </div>
      </div>
    </section>
  )
}

export default Services
