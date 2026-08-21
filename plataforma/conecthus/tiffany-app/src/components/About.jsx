import { Shield, Cpu, Users, Globe } from 'lucide-react'
import './About.css'

function About() {
  return (
    <section id="sobre" className="section section-dark">
      <div className="container">
        <div className="about-grid">
          <div className="about-content">
            <span className="badge">Sobre Nós</span>
            <h2>Tecnologia brasileira para <span className="highlight">impulsionar seu negócio</span></h2>
            <p>
              A Patria Technology nasceu com a missão de democratizar o acesso à
              inteligência artificial para empresas de todos os portes. Combinamos
              tecnologia de ponta com conhecimento profundo do mercado brasileiro
              para entregar resultados reais.
            </p>
            <p>
              Nossa plataforma utiliza modelos avançados de IA para automatizar
              processos de marketing digital, permitindo que você foque no que
              realmente importa: fazer seu negócio crescer.
            </p>
          </div>
          <div className="about-features">
            <div className="about-feature">
              <div className="about-feature-icon">
                <Cpu size={24} />
              </div>
              <div>
                <h4>IA de Última Geração</h4>
                <p>Modelos treinados especificamente para o mercado brasileiro.</p>
              </div>
            </div>
            <div className="about-feature">
              <div className="about-feature-icon">
                <Shield size={24} />
              </div>
              <div>
                <h4>Segurança Total</h4>
                <p>Seus dados protegidos com criptografia de ponta a ponta.</p>
              </div>
            </div>
            <div className="about-feature">
              <div className="about-feature-icon">
                <Users size={24} />
              </div>
              <div>
                <h4>Suporte Humano</h4>
                <p>Time dedicado para garantir o sucesso da sua estratégia.</p>
              </div>
            </div>
            <div className="about-feature">
              <div className="about-feature-icon">
                <Globe size={24} />
              </div>
              <div>
                <h4>Multi-plataforma</h4>
                <p>Instagram, Facebook, LinkedIn, X, TikTok e muito mais.</p>
              </div>
            </div>
          </div>
        </div>

        <div className="founders-section">
          <h3>Sócios Fundadores</h3>
          <div className="founders-grid">
            <div className="founder-card">
              <div className="founder-avatar">AL</div>
              <h4>Aarão Melo Lopes</h4>
              <span className="founder-role">CTO — Diretor de Tecnologia</span>
            </div>
            <div className="founder-card">
              <div className="founder-avatar">CG</div>
              <h4>Carlos Daniel Sousa Gomes</h4>
              <span className="founder-role">CEO — Diretor de Negócios</span>
            </div>
            <div className="founder-card">
              <div className="founder-avatar">PC</div>
              <h4>Patrícia Silva Cunha</h4>
              <span className="founder-role">Diretora de Marketing</span>
            </div>
          </div>
        </div>
      </div>
    </section>
  )
}

export default About
