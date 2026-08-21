import { useState } from 'react'
import { Menu, X } from 'lucide-react'
import './Header.css'

function Header() {
  const [menuOpen, setMenuOpen] = useState(false)

  return (
    <header className="header">
      <div className="container header-inner">
        <a href="#" className="logo">
          <span className="logo-icon">P</span>
          <span className="logo-text">
            Patria <span className="logo-accent">Technology</span>
          </span>
        </a>

        <nav className={`nav ${menuOpen ? 'nav-open' : ''}`}>
          <a href="#servicos" onClick={() => setMenuOpen(false)}>Serviços</a>
          <a href="#sobre" onClick={() => setMenuOpen(false)}>Sobre</a>
          <a href="#como-funciona" onClick={() => setMenuOpen(false)}>Como Funciona</a>
          <a href="#resultados" onClick={() => setMenuOpen(false)}>Resultados</a>
          <a href="#planos" onClick={() => setMenuOpen(false)}>Planos</a>
          <a href="#contato" onClick={() => setMenuOpen(false)}>Contato</a>
          <a
            href={import.meta.env.VITE_APP_REGISTER_URL || 'https://app.patriatechnology.com/register'}
            className="btn btn-primary nav-cta"
            target="_blank"
            rel="noopener noreferrer"
            onClick={() => setMenuOpen(false)}
          >
            Começar
          </a>
        </nav>

        <button
          className="menu-toggle"
          onClick={() => setMenuOpen(!menuOpen)}
          aria-label="Menu"
        >
          {menuOpen ? <X size={24} /> : <Menu size={24} />}
        </button>
      </div>
    </header>
  )
}

export default Header
