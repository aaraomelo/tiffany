# Build do erp-app (SPA) + erp-api (NestJS) num único container.
# Contexto de build: raiz do repo patria-erp.

# 1) Build do SPA
FROM node:20-alpine AS app
WORKDIR /app
COPY erp-app/package*.json ./
RUN npm ci
COPY erp-app/ ./
RUN npm run build

# 2) Build da API
FROM node:20-alpine AS api
WORKDIR /api
COPY erp-api/package*.json ./
RUN npm ci
COPY erp-api/ ./
RUN npx prisma generate && npm run build

# 3) Runtime
FROM node:20-alpine
WORKDIR /app
ENV NODE_ENV=production
COPY --from=api /api/node_modules ./node_modules
COPY --from=api /api/dist ./dist
COPY --from=api /api/prisma ./prisma
COPY --from=api /api/prisma.config.ts ./prisma.config.ts
COPY --from=api /api/package.json ./package.json
# main.js fica em dist/src/, então rootPath = dist/public (join(__dirname,'..','public'))
COPY --from=app /app/dist ./dist/public
EXPOSE 8080
# Aplica migrations e sobe a API (que também serve o SPA em public/)
CMD ["sh", "-c", "npx prisma migrate deploy && node dist/src/main.js"]
