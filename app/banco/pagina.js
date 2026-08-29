const st = document.getElementById('bk-status')
const btn = document.getElementById('bk-btn')
if (st) st.textContent = 'js montou o DOM (fetch + wasm MOVE)'
if (btn) btn.addEventListener('click', () => {
  if (st) st.textContent = 'clicou — idempotente: ' + Date.now()
})
