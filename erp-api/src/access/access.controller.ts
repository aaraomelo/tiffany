import {
  Body,
  Controller,
  Delete,
  Get,
  Param,
  ParseUUIDPipe,
  Patch,
  Post,
  Put,
} from '@nestjs/common';
import {
  CurrentUser,
  type AuthUser,
} from '../common/decorators/current-user.decorator';
import { AccessService } from './access.service';
import { CheckPolicies } from './check-policies.decorator';
import {
  AssignRolesDto,
  CreateRoleDto,
  UpdateRoleDto,
} from './dto/access.dto';

@Controller('access')
export class AccessController {
  constructor(private readonly service: AccessService) {}

  // Abilities do usuário logado — qualquer autenticado pode ler as próprias.
  @Get('me')
  me(@CurrentUser() user: AuthUser) {
    return this.service.me(user.sub);
  }

  // Catálogo de ações/recursos para a tela montar a matriz.
  @Get('catalog')
  @CheckPolicies({ action: 'read', subject: 'Role' })
  catalog() {
    return this.service.catalog();
  }

  // -------- perfis (gestão exige permissão sobre Role) --------
  @Get('roles')
  @CheckPolicies({ action: 'read', subject: 'Role' })
  listRoles() {
    return this.service.listRoles();
  }

  @Get('roles/:id')
  @CheckPolicies({ action: 'read', subject: 'Role' })
  getRole(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.getRole(id);
  }

  @Post('roles')
  @CheckPolicies({ action: 'create', subject: 'Role' })
  createRole(@Body() dto: CreateRoleDto) {
    return this.service.createRole(dto);
  }

  @Patch('roles/:id')
  @CheckPolicies({ action: 'update', subject: 'Role' })
  updateRole(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: UpdateRoleDto,
  ) {
    return this.service.updateRole(id, dto);
  }

  @Delete('roles/:id')
  @CheckPolicies({ action: 'delete', subject: 'Role' })
  deleteRole(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.deleteRole(id);
  }

  // -------- usuários e atribuição --------
  @Get('users')
  @CheckPolicies({ action: 'read', subject: 'User' })
  listUsers() {
    return this.service.listUsers();
  }

  @Put('users/:userId/roles')
  @CheckPolicies({ action: 'update', subject: 'User' })
  setUserRoles(
    @Param('userId', ParseUUIDPipe) userId: string,
    @Body() dto: AssignRolesDto,
  ) {
    return this.service.setUserRoles(userId, dto);
  }
}
