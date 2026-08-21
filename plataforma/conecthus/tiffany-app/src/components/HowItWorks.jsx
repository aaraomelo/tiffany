import { MessageSquare, Settings, Rocket, BarChart } from 'lucide-react'
import './HowItWorks.css'

const steps = [
  {
    icon: MessageSquare,
    number: '01',
    title: 'Conte sua história',
    description: 'Compartilhe os objetivos da sua marca, público-alvo e tom de voz desejado.',
  },
  {
    icon: Settings,
    number: '02',
    title: 'Configuramos a IA',
    description: 'Nossa equipe configura a plataforma personalizada para o seu negócio.',
  },
  {
    icon: Rocket,
    number: '03',
    title: 'Automação ativa',
    description: 'A IA começa a gerar conteúdo, agendar publicações e otimizar campanhas.',
  },
  {
    icon: BarChart,
    number: '04',
    title: 'Resultados reais',
    description: 'Acompanhe o crescimento com relatórios detalhados e insights acionáveis.',
  },
]

function HowItWorks() {
  return (
    <section id="como-funciona" className="section">
      <div className="container">
        <div className="section-header">
          <span className="badge">Como Funciona</span>
          <h2>Simples, rápido e <span className="highlight">eficiente</span></h2>
          <p>
            Em 4 passos simples, sua presença digital será transformada com o poder da IA.
          </p>
        </div>
        <div className="steps-grid">
          {steps.map((step, index) => (
            <div key={index} className="step-card">
              <div className="step-number">{step.number}</div>
              <div className="step-icon">
                <step.icon size={28} />
              </div>
              <h3>{step.title}</h3>
              <p>{step.description}</p>
              {index < steps.length - 1 && <div className="step-connector" />}
            </div>
          ))}
        </div>
      </div>
    </section>
  )
}

export default HowItWorks
