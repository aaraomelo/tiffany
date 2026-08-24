import { NestFactory } from '@nestjs/core';
import { ValidationPipe } from '@nestjs/common';
import { SwaggerModule, DocumentBuilder } from '@nestjs/swagger';
import * as bodyParser from 'body-parser';
import { AppModule } from './app.module';
import { RequestContextInterceptor } from './request-context.interceptor';

async function bootstrap() {
  const app = await NestFactory.create(AppModule);
  // Aumenta limite pra acomodar adjacency JSON grande (n=2000 ≈ 16MB)
  app.use(bodyParser.json({ limit: '32mb' }));
  app.use(bodyParser.urlencoded({ extended: true, limit: '32mb' }));
  app.useGlobalInterceptors(app.get(RequestContextInterceptor));
  app.useGlobalPipes(new ValidationPipe({ whitelist: true, transform: true }));
  app.enableCors();

  const config = new DocumentBuilder()
    .setTitle('Patria NCO API')
    .setDescription('API pública de otimização combinatorial neural --- max-cut, k-coloring, structural balance, MIS')
    .setVersion('1.0')
    .addApiKey({ type: 'apiKey', name: 'X-API-Key', in: 'header' }, 'api-key')
    .build();
  const document = SwaggerModule.createDocument(app, config);
  // Filtra rotas públicas NCO (oculta endpoints internos: /api/auth, /api/billing, /api/patricia, etc)
  document.paths = Object.fromEntries(
    Object.entries(document.paths).filter(([path]) => path.startsWith('/api/nco')),
  );
  SwaggerModule.setup('api/docs', app, document);

  // A porta esteve COZIDA em 8080 até 21/08/2026, e o docker é que a disfarçava: os três
  // ambientes corriam a mesma imagem e o mapeamento (8080/8081/8082 -> 8080) fazia o resto.
  // Sem docker não há tradução, e três processos não cabem na mesma porta. Agora vem do
  // ambiente, com o 8080 a manter-se como omissão para nada mudar de dentro do container.
  const port = Number(process.env.PORT ?? 8080);
  await app.listen(port);
  console.log(`patria-api running on port ${port}`);
  console.log(`Swagger: http://localhost:${port}/api/docs`);
}
bootstrap();
