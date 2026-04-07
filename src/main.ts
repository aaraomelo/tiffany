import { NestFactory } from '@nestjs/core';
import { AppModule } from './app.module';
import { RequestContextInterceptor } from './request-context.interceptor';

async function bootstrap() {
  const app = await NestFactory.create(AppModule);
  app.useGlobalInterceptors(new RequestContextInterceptor());
  app.enableCors();
  await app.listen(8080);
  console.log('patria-api running on port 8080');
}
bootstrap();
