import {
  Body,
  Controller,
  Headers,
  HttpCode,
  Param,
  ParseUUIDPipe,
  Post,
} from '@nestjs/common';
import { Public } from '../common/decorators/public.decorator';
import { RequiresModule } from '../common/decorators/requires-module.decorator';
import { PaymentService } from '../payment/payment.service';
import { PrismaService } from '../prisma/prisma.service';
import { CheckoutService } from './checkout.service';
import { CreatePixCheckoutDto } from './dto/checkout.dto';

@RequiresModule('checkout')
@Controller()
export class CheckoutController {
  constructor(
    private readonly checkout: CheckoutService,
    private readonly payments: PaymentService,
    private readonly prisma: PrismaService,
  ) {}

  @Post('checkout/pix')
  createPix(@Body() dto: CreatePixCheckoutDto) {
    return this.checkout.createPix(dto);
  }

  /// Simulação local de confirmação (modo stub).
  @Post('checkout/simulate-confirm/:paymentId')
  simulate(@Param('paymentId', ParseUUIDPipe) paymentId: string) {
    return this.checkout.simulateConfirm(paymentId);
  }

  /// Webhook público do PagBank.
  /// Padrão Patria: grava o evento bruto antes de processar (idempotência via referenceId).
  @Public()
  @HttpCode(200)
  @Post('webhooks/pagbank')
  async webhook(
    @Body() body: unknown,
    @Headers('x-authenticity-token') _token?: string,
  ) {
    const payload = body as {
      id?: string;
      reference_id?: string;
      charges?: Array<{
        id?: string;
        status?: string;
        payment_response?: { code?: string };
      }>;
    };

    const event = await this.prisma.billingEvent.create({
      data: {
        source: 'pagbank',
        referenceId: payload.reference_id ?? null,
        eventType: 'order.status',
        payload: payload as never,
      },
    });

    try {
      const charge = payload.charges?.find((c) => c.status === 'PAID');
      if (charge?.id) {
        const payment = await this.prisma.payment.findFirst({
          where: { pagbankChargeId: charge.id },
        });
        if (payment) {
          await this.payments.confirmPayment(payment.id);
        }
      }
      await this.prisma.billingEvent.update({
        where: { id: event.id },
        data: { processed: true, processedAt: new Date() },
      });
    } catch (err) {
      await this.prisma.billingEvent.update({
        where: { id: event.id },
        data: { errorMessage: (err as Error).message },
      });
      throw err;
    }
    return { ok: true };
  }
}
