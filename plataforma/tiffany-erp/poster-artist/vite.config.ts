import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

// https://vite.dev/config/
export default defineConfig({
  plugins: [react()],
  // Mesma proteção do erp-app: força uma única cópia (do node_modules do
  // projeto), senão o Vite sobe a árvore e pega um @mui/styled-engine de um
  // node_modules pai que não enxerga o Emotion, quebrando o build.
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
        manualChunks(id) {
          if (id.includes('node_modules')) {
            if (id.includes('@mui') || id.includes('@emotion')) return 'mui'
            if (id.includes('pdf-lib') || id.includes('fontkit') || id.includes('opentype'))
              return 'pdf'
            if (id.includes('react') || id.includes('scheduler')) return 'react-vendor'
            return 'vendor'
          }
        },
      },
    },
  },
  server: {
    // 5174 é o erp-app; usamos 5175 pra rodar lado a lado.
    port: 5175,
  },
})
