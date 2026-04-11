import { ArrowRight, Sparkles, Zap, TrendingUp } from 'lucide-react'
import './Hero.css'

function Hero() {
  return (
    <section className="hero-section">
      <div className="hero-bg">
        <div className="hero-gradient-orb orb-1" />
        <div className="hero-gradient-orb orb-2" />
        <div className="hero-grid" />
      </div>
      <div className="container hero-content">
        <div className="hero-badge">
          <Sparkles size={16} />
          Automação inteligente com IA
        </div>
        <h1 className="hero-title">
          Transforme suas redes sociais com o poder da{' '}
          <span className="highlight">Inteligência Artificial</span>
        </h1>
        <p className="hero-subtitle">
          Automatize publicações, gere conteúdo de alta qualidade e otimize seu
          tráfego pago. Tudo isso com tecnologia de ponta que trabalha 24h por
          você.
        </p>
        <div className="hero-buttons">
          <a
            href={import.meta.env.VITE_APP_REGISTER_URL || 'https://app.patriatechnology.com/register'}
            className="btn btn-primary btn-lg"
            target="_blank"
            rel="noopener noreferrer"
          >
            Comece Agora <ArrowRight size={20} />
          </a>
          <a href="#como-funciona" className="btn btn-secondary btn-lg">
            Saiba Mais
          </a>
        </div>
        <div className="hero-stats">
          <div className="hero-stat">
            <Zap size={20} className="stat-icon" />
            <div>
              <strong>10x</strong>
              <span>mais rápido</span>
            </div>
          </div>
          <div className="hero-stat">
            <TrendingUp size={20} className="stat-icon" />
            <div>
              <strong>300%</strong>
              <span>mais engajamento</span>
            </div>
          </div>
          <div className="hero-stat">
            <Sparkles size={20} className="stat-icon" />
            <div>
              <strong>24/7</strong>
              <span>automação ativa</span>
            </div>
          </div>
        </div>
      </div>
    </section>
  )
}

export default Hero
