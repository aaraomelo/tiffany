import { Globe, Briefcase, MessageCircle } from 'lucide-react'
import './Footer.css'

function Footer() {
  return (
    <footer className="footer">
      <div className="container">
        <div className="footer-grid">
          <div className="footer-brand">
            <div className="logo">
              <span className="logo-icon">P</span>
              <span className="logo-text">
                Patria <span className="logo-accent">Technology</span>
              </span>
            </div>
            <p>
              Transformando negócios com o poder da inteligência artificial.
              Automação inteligente para redes sociais, conteúdo e tráfego pago.
            </p>
            <div className="footer-socials">
              <a href="#" aria-label="Instagram"><Globe size={20} /></a>
              <a href="#" aria-label="LinkedIn"><Briefcase size={20} /></a>
              <a href="#" aria-label="X"><MessageCircle size={20} /></a>
            </div>
          </div>
          <div className="footer-links">
            <h4>Empresa</h4>
            <a href="#sobre">Sobre Nós</a>
            <a href="#servicos">Serviços</a>
            <a href="#resultados">Resultados</a>
          </div>
          <div className="footer-links">
            <h4>Serviços</h4>
            <a href="#servicos">Automação de Redes</a>
            <a href="#servicos">Geração de Conteúdo</a>
            <a href="#servicos">Tráfego Pago</a>
            <a href="#servicos">Análise de Dados</a>
          </div>
          <div className="footer-links">
            <h4>Suporte</h4>
            <a href="#">Central de Ajuda</a>
            <a href="#">Termos de Uso</a>
            <a href="#">Privacidade</a>
          </div>
        </div>
        <div className="footer-bottom">
          <p>&copy; 2026 Patria Technology. Todos os direitos reservados.</p>
        </div>
      </div>
    </footer>
  )
}

export default Footer
