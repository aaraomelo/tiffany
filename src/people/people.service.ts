import { Injectable, NotFoundException } from '@nestjs/common';
import { PrismaService } from '../prisma.service';

const contactSelect = {
  channelType: true,
  remoteId: true,
  displayName: true,
  phone: true,
};

@Injectable()
export class PeopleService {
  constructor(private readonly prisma: PrismaService) {}

  async search(q: string, tenantId?: string) {
    return this.prisma.person.findMany({
      where: {
        ...(tenantId ? { tenantId } : {}),
        OR: [
          { name: { contains: q, mode: 'insensitive' } },
          { email: { contains: q, mode: 'insensitive' } },
          { phone: { contains: q, mode: 'insensitive' } },
        ],
      },
      include: { contacts: { select: contactSelect } },
      orderBy: { name: 'asc' },
    });
  }

  async findById(id: string, tenantId: string) {
    const person = await this.prisma.person.findFirst({
      where: { id, tenantId },
      include: { contacts: { select: contactSelect } },
    });
    if (!person) throw new NotFoundException(`Person ${id} not found`);
    return person;
  }
}
