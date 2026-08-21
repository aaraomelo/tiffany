import Header from './components/Header'
import Hero from './components/Hero'
import Services from './components/Services'
import About from './components/About'
import HowItWorks from './components/HowItWorks'
import Results from './components/Results'
import Pricing from './components/Pricing'
import Contact from './components/Contact'
import Footer from './components/Footer'
import './App.css'

function App() {
  return (
    <div className="app">
      <Header />
      <Hero />
      <Services />
      <About />
      <HowItWorks />
      <Results />
      <Pricing />
      <Contact />
      <Footer />
    </div>
  )
}

export default App
