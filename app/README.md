# Reino Dourado — app Vite local

A arte-conceito do **Reino Dourado (Golden Kingdom)** empacotada num app web local: a corte, o elenco, os mundos,
as insígnias, a cena 2D-em-3D, a dinâmica animada e o trailer — **30 peças rasterizadas na GPU local** (NVIDIA
GTX 1650) em 6 kernels PTX escritos à mão, mais o GDD e o enredo em PDF.

E não só imagem: a seção **Os substratos** roda **ao vivo no navegador** o *filtro do colapso* das refs
(`dep = (rel≠0) & existe`) em **WASM** (a VM de pilha, na CPU — `WebAssembly.instantiate`) e **GLSL** (WebGL2, uma
coluna por aresta, na GPU), sobre arestas reais do grafo, mostrando que WASM ≡ GLSL ≡ o esperado.

## Rodar

```bash
cd app
npm install
npm run dev        # abre http://localhost:5173
```

`npm run build` gera `dist/`; `npm run preview` serve o build.

## Como funciona

- **Nada é desenhado — tudo é computado.** Cada figura é o campo da matemática do operador, rasterizado pixel a
  pixel no metal. Os kernels vivem em `../sandbox/tecnicas/*_gpu.py` e `../laboratorio_ptx.py`.
- **`vite.config.js`** aponta `publicDir` para `../figuras`, então o app serve os assets full-res direto
  (`/reino/*.png`, `/reino/dinamica/*.gif`, `/reino/trailer.gif`, `/docs/*.pdf`) — sem cópia, sem base64.
- **`src/manifesto.json`** é o único ponto de verdade (as peças, as réguas, o trailer, os docs). É gerado por um
  nó do grafo — `../sandbox/tecnicas/app_manifesto.py` — que **certifica a completude** (toda peça declarada
  existe; senão o manifesto não fecha). `src/main.js` só o consome e monta a UI (data-driven).

## Regenerar a arte / o manifesto

```bash
cd ..
python3 sandbox/tecnicas/retrato_operador_gpu.py    # o elenco
python3 sandbox/tecnicas/mapas_galaxias_gpu.py      # os mundos
python3 sandbox/tecnicas/corte_real_gpu.py          # a corte
python3 sandbox/tecnicas/insignias_gpu.py           # as insígnias
python3 sandbox/tecnicas/cena_gpu.py                # a cena 2D-em-3D
python3 sandbox/tecnicas/dinamica_gpu.py            # a dinâmica (GIF)
python3 sandbox/tecnicas/trailer_gpu.py             # o trailer
python3 sandbox/tecnicas/app_manifesto.py           # regenera app/src/manifesto.json + assets/figuras/docs/
python3 sandbox/tecnicas/app_substratos.py          # regenera assets/figuras/wasm/filtro.wasm + app/src/substratos.json (WASM/GLSL)
```

Cada módulo é certificado pelo Formalizador (resíduo 0) e rasteriza no metal via `laboratorio_ptx.py` (PTX à mão,
JIT PTX→SASS pelo driver, sem toolkit CUDA).
