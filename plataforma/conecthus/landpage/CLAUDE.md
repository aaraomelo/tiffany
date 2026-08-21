# Instruções para Claude Code — landpage

## Stack

- React 19 + Vite 5
- Lucide React (ícones)
- CSS puro (sem Tailwind, sem styled-components)
- Node 22

## Estrutura

```
src/
├── App.jsx          ← componente principal, importa todos os outros
├── App.css
├── index.css        ← estilos globais, variáveis CSS
├── main.jsx         ← entry point
├── components/      ← cada componente tem .jsx + .css
│   ├── Header.jsx / Header.css
│   ├── Hero.jsx / Hero.css
│   ├── Services.jsx / Services.css
│   ├── About.jsx / About.css
│   ├── HowItWorks.jsx / HowItWorks.css
│   ├── Results.jsx / Results.css
│   ├── Pricing.jsx / Pricing.css
│   └── Footer.jsx / Footer.css
└── assets/          ← imagens, SVGs
```

## Convenções

- Um componente = um arquivo .jsx + um arquivo .css
- Importar e registrar todo componente novo em `App.jsx`
- Usar classes CSS do projeto (variáveis em `index.css`: `--primary`, `--accent`, etc.)
- Usar Lucide React para ícones: `import { Phone, Mail } from 'lucide-react'`
- Componentes em PascalCase, arquivos em PascalCase
- NÃO usar TypeScript (é JavaScript/JSX puro)

## Docker

O build roda em container nginx:alpine.
```
/usr/share/nginx/html/   ← dist/ é copiado aqui
```

O Vite faz build para `dist/`. O Dockerfile copia `dist/` para o nginx.

## Deploy

- Push em `develop` → DEV (dev.patriatechnology.com)
- Push em `homolog` → HOMOLOG (homolog.patriatechnology.com)
- Push em `main` → PROD (patriatechnology.com)

## Estilo

- Seguir o padrão visual existente (seções com `.section`, containers com `.container`)
- Cores via variáveis CSS (`--primary: #6C3AED`, `--accent`, etc.)
- Responsivo (mobile-first, breakpoints em 768px e 1024px)
- Badges: `.badge` com ícone + texto

## OBRIGATÓRIO: Validação em Formulários

Todo formulário DEVE ter validação no frontend que espelhe as regras do backend.

### Regras

1. **Validar antes de enviar** — não fazer POST se o formulário é inválido
2. **Mensagens de erro inline** — abaixo de cada campo com erro
3. **Validações mínimas:**
   - Campos obrigatórios: verificar se não está vazio
   - Email: regex simples `/^[^\s@]+@[^\s@]+\.[^\s@]+$/`
   - Telefone: mínimo 10 dígitos
   - Números com range: verificar min/max (ex: nota 1-5)
   - Checkbox obrigatório: verificar se está marcado

### Padrão de implementação

```jsx
const [errors, setErrors] = useState({});

function validate() {
  const errs = {};
  if (!form.nome?.trim()) errs.nome = 'Nome é obrigatório';
  if (!form.email?.trim()) errs.email = 'Email é obrigatório';
  else if (!/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(form.email)) errs.email = 'Email inválido';
  if (form.nota !== undefined && (form.nota < 1 || form.nota > 5)) errs.nota = 'Nota deve ser entre 1 e 5';
  setErrors(errs);
  return Object.keys(errs).length === 0;
}

function handleSubmit(e) {
  e.preventDefault();
  if (!validate()) return;
  // POST para API...
}
```

### CSS para erros

```css
.field-error {
  color: #ef4444;
  font-size: 0.85rem;
  margin-top: 0.25rem;
}

.input-error {
  border-color: #ef4444;
}
```

### Regra importante

As validações do frontend devem ESPELHAR as do backend. Se o backend exige `@IsEmail()` e `@Min(1) @Max(5)`, o frontend deve validar o mesmo. Consulte o endpoint da API para saber as regras.
