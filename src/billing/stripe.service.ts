import { Injectable, BadRequestException, InternalServerErrorException } from '@nestjs/common';
import Stripe from 'stripe';

@Injectable()
export class StripeService {
  private readonly stripe: Stripe;

  constructor() {
    this.stripe = new Stripe(process.env.STRIPE_SECRET_KEY ?? '', {
      apiVersion: '2025-02-24.acacia',
    });
  }

  async createCustomer(
    email: string,
    name: string,
    metadata: Record<string, string>,
  ): Promise<Stripe.Customer> {
    try {
      return await this.stripe.customers.create({ email, name, metadata });
    } catch (err) {
      this.handleStripeError(err);
    }
  }

  async createSubscription(
    customerId: string,
    priceId: string,
    trialEnd?: number,
  ): Promise<Stripe.Subscription> {
    try {
      return await this.stripe.subscriptions.create({
        customer: customerId,
        items: [{ price: priceId }],
        ...(trialEnd ? { trial_end: trialEnd } : {}),
      });
    } catch (err) {
      this.handleStripeError(err);
    }
  }

  async cancelSubscription(
    subscriptionId: string,
    atPeriodEnd = true,
  ): Promise<Stripe.Subscription> {
    try {
      if (atPeriodEnd) {
        return await this.stripe.subscriptions.update(subscriptionId, {
          cancel_at_period_end: true,
        });
      }
      return await this.stripe.subscriptions.cancel(subscriptionId);
    } catch (err) {
      this.handleStripeError(err);
    }
  }

  async retrieveSubscription(subscriptionId: string): Promise<Stripe.Subscription> {
    try {
      return await this.stripe.subscriptions.retrieve(subscriptionId);
    } catch (err) {
      this.handleStripeError(err);
    }
  }

  async retrieveCustomer(customerId: string): Promise<Stripe.Customer> {
    try {
      const customer = await this.stripe.customers.retrieve(customerId);
      if (customer.deleted) {
        throw new BadRequestException(`Customer ${customerId} foi deletado no Stripe`);
      }
      return customer as Stripe.Customer;
    } catch (err) {
      this.handleStripeError(err);
    }
  }

  private handleStripeError(err: unknown): never {
    if (err instanceof Stripe.errors.StripeInvalidRequestError) {
      throw new BadRequestException(err.message);
    }
    if (err instanceof Stripe.errors.StripeError) {
      throw new InternalServerErrorException(`Stripe error: ${err.message}`);
    }
    throw err;
  }
}
