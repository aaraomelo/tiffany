import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

// https://vite.dev/config/
export default defineConfig({
  plugins: [react()],
  // Força uma única cópia (do node_modules do projeto). Sem isto, o Vite sobe
  // a árvore e pode pegar um @mui/styled-engine de um node_modules pai que não
  // enxerga o Emotion, quebrando o build.
  resolve: {
    dedupe: [
      'react',
      'react-dom',
      '@mui/material',
      '@mui/system',
      '@mui/styled-engine',
      '@emotion/react',
      '@emotion/styled',
    ],
  },
  build: {
    rollupOptions: {
      output: {
        // Separa o vendor em chunks estáveis (melhor cache entre deploys)
        manualChunks(id) {
          if (id.includes('node_modules')) {
            // pdf-lib/fontkit/opentype só são usados na geração de PDF (import
            // dinâmico) — chunk isolado pra não pesar o bundle inicial.
            if (id.includes('pdf-lib') || id.includes('fontkit') || id.includes('opentype'))
              return 'pdf'
            if (id.includes('@mui') || id.includes('@emotion')) return 'mui'
            if (id.includes('react') || id.includes('scheduler')) return 'react-vendor'
            return 'vendor'
          }
        },
      },
    },
  },
  server: {
    port: 5174,
    proxy: {
      '/api': {
        target: 'http://localhost:3001',
        changeOrigin: true,
      },
    },
  },
})
