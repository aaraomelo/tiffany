import { NestFactory } from '@nestjs/core';
import { AppModule } from './app.module';
import { RequestContextInterceptor } from './request-context.interceptor';
import { ApiKeyGuard } from './api-key.guard';

async function bootstrap() {
  const app = await NestFactory.create(AppModule);
  app.useGlobalInterceptors(new RequestContextInterceptor());
  app.useGlobalGuards(new ApiKeyGuard());
  app.enableCors();
  await app.listen(8080);
  console.log('patria-api running on port 8080');
}
bootstrap();
