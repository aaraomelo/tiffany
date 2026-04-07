FROM node:20-alpine
WORKDIR /app
COPY package.json package-lock.json ./
RUN npm ci --omit=dev
COPY dist/ ./dist/
COPY public/ ./public/
RUN mkdir -p /app/data
VOLUME ["/app/data"]
EXPOSE 8080
CMD ["node", "dist/main"]
