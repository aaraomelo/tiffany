FROM node:20-slim
RUN apt-get update -y && apt-get install -y openssl openssh-client git && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY package.json package-lock.json ./
COPY prisma/ ./prisma/
RUN npm ci --omit=dev && npx prisma generate
COPY dist/ ./dist/
COPY public/ ./public/
COPY PRODUCT.md ./PRODUCT.md
RUN mkdir -p /app/data
VOLUME ["/app/data"]
EXPOSE 8080
CMD ["node", "dist/main"]
