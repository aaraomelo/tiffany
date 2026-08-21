import { useState } from 'react'
import './Contact.css'

const EMAIL_REGEX = /^[^\s@]+@[^\s@]+\.[^\s@]+$/
const PHONE_REGEX = /^[\d\s()\-+]{7,20}$/

function validate(form, fields) {
  const errs = {}
  const check = fields ? fields : ['name', 'email', 'phone', 'message']

  if (check.includes('name') && !form.name.trim()) {
    errs.name = 'Nome é obrigatório.'
  }
  if (check.includes('email')) {
    if (!form.email.trim()) {
      errs.email = 'E-mail é obrigatório.'
    } else if (!EMAIL_REGEX.test(form.email.trim())) {
      errs.email = 'Informe um e-mail válido.'
    }
  }
  if (check.includes('phone') && form.phone.trim() && !PHONE_REGEX.test(form.phone.trim())) {
    errs.phone = 'Formato de telefone inválido.'
  }
  if (check.includes('message') && !form.message.trim()) {
    errs.message = 'Mensagem é obrigatória.'
  }
  return errs
}

function Contact() {
  const [form, setForm] = useState({ name: '', email: '', phone: '', message: '', whatsappConsent: false })
  const [status, setStatus] = useState(null) // null | 'loading' | 'success' | 'error'
  const [errors, setErrors] = useState({})

  function handleChange(e) {
    const { name, value, type, checked } = e.target
    setForm({ ...form, [name]: type === 'checkbox' ? checked : value })
  }

  function handleBlur(e) {
    const { name } = e.target
    const fieldErrors = validate(form, [name])
    setErrors(prev => ({ ...prev, [name]: fieldErrors[name] || undefined }))
  }

  async function handleSubmit(e) {
    e.preventDefault()
    const allErrors = validate(form)
    if (Object.keys(allErrors).length > 0) {
      setErrors(allErrors)
      return
    }
    setErrors({})
    setStatus('loading')
    try {
      const res = await fetch('/api/contacts', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(form),
      })
      if (!res.ok) throw new Error()
      setStatus('success')
      setForm({ name: '', email: '', phone: '', message: '', whatsappConsent: false })
    } catch {
      setStatus('error')
    }
  }

  return (
    <section id="contato" className="section contact-section">
      <div className="container">
        <div className="section-header">
          <span className="badge">Contato</span>
          <h2>Fale com a <span className="highlight">nossa equipe</span></h2>
          <p>Tire suas dúvidas ou solicite uma proposta personalizada.</p>
        </div>
        <div className="contact-card">
          <form className="contact-form" onSubmit={handleSubmit} noValidate>
            <div className={`contact-field${errors.name ? ' has-error' : ''}`}>
              <label htmlFor="contact-name">Nome</label>
              <input
                id="contact-name"
                type="text"
                name="name"
                placeholder="Seu nome completo"
                value={form.name}
                onChange={handleChange}
                onBlur={handleBlur}
                disabled={status === 'loading'}
              />
              {errors.name && <span className="contact-error-msg">{errors.name}</span>}
            </div>
            <div className={`contact-field${errors.email ? ' has-error' : ''}`}>
              <label htmlFor="contact-email">E-mail</label>
              <input
                id="contact-email"
                type="email"
                name="email"
                placeholder="seu@email.com"
                value={form.email}
                onChange={handleChange}
                onBlur={handleBlur}
                disabled={status === 'loading'}
              />
              {errors.email && <span className="contact-error-msg">{errors.email}</span>}
            </div>
            <div className={`contact-field${errors.phone ? ' has-error' : ''}`}>
              <label htmlFor="contact-phone">Telefone <span className="contact-optional">(opcional)</span></label>
              <input
                id="contact-phone"
                type="tel"
                name="phone"
                placeholder="(11) 90000-0000"
                value={form.phone}
                onChange={handleChange}
                onBlur={handleBlur}
                disabled={status === 'loading'}
              />
              {errors.phone && <span className="contact-error-msg">{errors.phone}</span>}
            </div>
            <label className="contact-checkbox">
              <input
                type="checkbox"
                name="whatsappConsent"
                checked={form.whatsappConsent}
                onChange={handleChange}
                disabled={status === 'loading'}
              />
              Aceito retorno via WhatsApp
            </label>
            <div className={`contact-field${errors.message ? ' has-error' : ''}`}>
              <label htmlFor="contact-message">Mensagem</label>
              <textarea
                id="contact-message"
                name="message"
                placeholder="Como podemos ajudar?"
                rows={5}
                value={form.message}
                onChange={handleChange}
                onBlur={handleBlur}
                disabled={status === 'loading'}
              />
              {errors.message && <span className="contact-error-msg">{errors.message}</span>}
            </div>
            {status === 'success' && (
              <p className="contact-feedback contact-success">
                Mensagem enviada com sucesso! Entraremos em contato em breve.
              </p>
            )}
            {status === 'error' && (
              <p className="contact-feedback contact-error">
                Ocorreu um erro ao enviar. Tente novamente.
              </p>
            )}
            <button
              type="submit"
              className="btn btn-primary contact-submit"
              disabled={status === 'loading'}
            >
              {status === 'loading' ? 'Enviando...' : 'Enviar mensagem'}
            </button>
          </form>
        </div>
      </div>
    </section>
  )
}

export default Contact
