export const PATRICIA_TOOLS = [
  {
    name: 'status',
    description: 'Consultar status geral (tarefas, projetos, pendências). USE SEMPRE antes de responder sobre status.',
    input_schema: { type: 'object' as const, properties: {} },
  },
  {
    name: 'create_task',
    description: 'Criar tarefa simples (alteração em um repo)',
    input_schema: {
      type: 'object' as const,
      properties: {
        command: { type: 'string', description: 'Título curto da tarefa' },
        description: { type: 'string', description: 'Detalhes da tarefa' },
      },
      required: ['command'],
    },
  },
  {
    name: 'create_project',
    description: 'Criar projeto complexo (múltiplas etapas/repos)',
    input_schema: {
      type: 'object' as const,
      properties: {
        name: { type: 'string', description: 'Nome do projeto' },
        description: { type: 'string', description: 'Descrição do projeto' },
      },
      required: ['name'],
    },
  },
  {
    name: 'approve_task',
    description: 'Aprovar plano de uma tarefa',
    input_schema: {
      type: 'object' as const,
      properties: {
        taskId: { type: 'string' },
      },
    },
  },
  {
    name: 'reject_task',
    description: 'Rejeitar plano com feedback',
    input_schema: {
      type: 'object' as const,
      properties: {
        taskId: { type: 'string' },
        feedback: { type: 'string', description: 'Motivo da rejeição' },
      },
    },
  },
  {
    name: 'approve_project',
    description: 'Aprovar plano do projeto para iniciar execução',
    input_schema: {
      type: 'object' as const,
      properties: {
        projectId: { type: 'string' },
      },
    },
  },
  {
    name: 'cancel_task',
    description: 'Cancelar uma tarefa',
    input_schema: {
      type: 'object' as const,
      properties: { taskId: { type: 'string' } },
    },
  },
  {
    name: 'cancel_project',
    description: 'Cancelar um projeto',
    input_schema: {
      type: 'object' as const,
      properties: { projectId: { type: 'string' } },
    },
  },
  {
    name: 'complete_project',
    description: 'Finalizar projeto (após diretor confirmar)',
    input_schema: {
      type: 'object' as const,
      properties: { projectId: { type: 'string' } },
    },
  },
  {
    name: 'promote',
    description: 'Promover tarefa/projeto para outro ambiente (homolog, prod)',
    input_schema: {
      type: 'object' as const,
      properties: {
        projectId: { type: 'string' },
        taskId: { type: 'string' },
        targetEnv: { type: 'string', enum: ['homolog', 'prod'] },
      },
      required: ['targetEnv'],
    },
  },
  {
    name: 'search_project',
    description: 'Buscar projeto por descrição',
    input_schema: {
      type: 'object' as const,
      properties: { q: { type: 'string' } },
      required: ['q'],
    },
  },
  {
    name: 'search_task',
    description: 'Buscar tarefa por descrição',
    input_schema: {
      type: 'object' as const,
      properties: { q: { type: 'string' } },
      required: ['q'],
    },
  },
  {
    name: 'ask_specialist',
    description: 'Fazer pergunta ao especialista técnico (Claude Code analisa o código real). Resposta chega por notificação. Use quando pedirem "pergunta pro técnico", "consulta o especialista".',
    input_schema: {
      type: 'object' as const,
      properties: { question: { type: 'string' } },
      required: ['question'],
    },
  },
  {
    name: 'diagnose',
    description: 'Diagnosticar problema técnico em um projeto',
    input_schema: {
      type: 'object' as const,
      properties: {
        projectId: { type: 'string' },
        question: { type: 'string' },
      },
      required: ['question'],
    },
  },
  {
    name: 'discuss',
    description: 'Enviar mensagem no planejamento de um projeto',
    input_schema: {
      type: 'object' as const,
      properties: {
        projectId: { type: 'string' },
        message: { type: 'string' },
      },
      required: ['message'],
    },
  },
  {
    name: 'pause',
    description: 'Pausar execução de projeto',
    input_schema: {
      type: 'object' as const,
      properties: { projectId: { type: 'string' } },
    },
  },
  {
    name: 'resume',
    description: 'Retomar execução de projeto pausado',
    input_schema: {
      type: 'object' as const,
      properties: { projectId: { type: 'string' } },
    },
  },
  {
    name: 'task_detail',
    description: 'Detalhes completos de uma tarefa',
    input_schema: {
      type: 'object' as const,
      properties: { taskId: { type: 'string' } },
    },
  },
  {
    name: 'project_detail',
    description: 'Detalhes completos de um projeto com subtarefas',
    input_schema: {
      type: 'object' as const,
      properties: { projectId: { type: 'string' } },
    },
  },
  {
    name: 'add_subtask',
    description: 'Adicionar subtarefa a um projeto existente',
    input_schema: {
      type: 'object' as const,
      properties: {
        projectId: { type: 'string' },
        command: { type: 'string' },
        description: { type: 'string' },
      },
      required: ['command'],
    },
  },
  {
    name: 'resolve_task',
    description: 'Marcar tarefa como concluída manualmente',
    input_schema: {
      type: 'object' as const,
      properties: { taskId: { type: 'string' } },
    },
  },
  {
    name: 'force_complete',
    description: 'Fechar projeto mesmo com subtarefas pendentes',
    input_schema: {
      type: 'object' as const,
      properties: { projectId: { type: 'string' } },
    },
  },
  {
    name: 'save_memory',
    description: 'Salvar informação importante para lembrar em conversas futuras. Use quando o diretor compartilhar decisões, preferências ou informações estratégicas.',
    input_schema: {
      type: 'object' as const,
      properties: {
        title: { type: 'string', description: 'Título curto da memória' },
        content: { type: 'string', description: 'Conteúdo detalhado' },
        category: {
          type: 'string',
          enum: ['decision', 'preference', 'project', 'technical', 'person'],
          description: 'Categoria da memória',
        },
        priority: {
          type: 'string',
          enum: ['long_term', 'short_term'],
          description: 'long_term = permanente, short_term = expira em 30 dias',
        },
      },
      required: ['title', 'content', 'category'],
    },
  },
  {
    name: 'forget_memory',
    description: 'Esquecer uma memória (não pode esquecer conhecimento base/core). Use quando o diretor disser para esquecer ou quando uma informação não é mais válida.',
    input_schema: {
      type: 'object' as const,
      properties: {
        title: { type: 'string', description: 'Título ou parte do título da memória a esquecer' },
      },
      required: ['title'],
    },
  },
  {
    name: 'open_specialist',
    description: 'Conectar o especialista técnico na conversa. O especialista assume e responde diretamente até o diretor dizer "fecha". Use quando o diretor pedir "chama o especialista", "conecta o técnico", "quero falar com o técnico".',
    input_schema: {
      type: 'object' as const,
      properties: {},
    },
  },
  {
    name: 'add_contact',
    description: 'Adicionar um contato (pessoa) no sistema. Use quando o diretor pedir para salvar contato de alguém.',
    input_schema: {
      type: 'object' as const,
      properties: {
        name: { type: 'string', description: 'Nome completo da pessoa' },
        phone: { type: 'string', description: 'Telefone com código do país (ex: +5511999999999)' },
        description: { type: 'string', description: 'Quem é essa pessoa em relação ao diretor (ex: irmã do Aarão, amigo da faculdade, sócio)' },
        profile: {
          type: 'string',
          enum: ['gestora', 'amiga', 'juridica', 'mentora', 'assistente'],
          description: 'Perfil de relacionamento',
        },
        role: {
          type: 'string',
          enum: ['director', 'member'],
          description: 'Papel (director = acesso total, member = limitado)',
        },
      },
      required: ['name', 'phone', 'profile'],
    },
  },
  {
    name: 'send_message',
    description: 'Enviar mensagem para um contato por WhatsApp ou Telegram. Use quando o diretor pedir para mandar mensagem para alguém.',
    input_schema: {
      type: 'object' as const,
      properties: {
        to: { type: 'string', description: 'Nome ou telefone do destinatário' },
        message: { type: 'string', description: 'Texto da mensagem' },
        channel: {
          type: 'string',
          enum: ['whatsapp', 'telegram'],
          description: 'Canal de envio (default: whatsapp)',
        },
      },
      required: ['to', 'message'],
    },
  },
  {
    name: 'check_contact',
    description: 'Verificar informações de um contato: quem é, última interação, mensagens recentes. Use SEMPRE quando perguntarem sobre outra pessoa, se respondeu, o que disse. NUNCA invente respostas de outras pessoas — consulte primeiro.',
    input_schema: {
      type: 'object' as const,
      properties: {
        name: { type: 'string', description: 'Nome, email ou telefone da pessoa (busca parcial)' },
      },
      required: ['name'],
    },
  },
  {
    name: 'update_contact',
    description: 'Atualizar informações de um contato existente (descrição, telefone, perfil). Use quando pedirem para atualizar dados de alguém.',
    input_schema: {
      type: 'object' as const,
      properties: {
        name: { type: 'string', description: 'Nome da pessoa a atualizar' },
        description: { type: 'string', description: 'Nova descrição (ex: irmã do Aarão, amigo da faculdade)' },
        phone: { type: 'string', description: 'Novo telefone (opcional)' },
        profile: { type: 'string', enum: ['gestora', 'amiga', 'juridica', 'mentora', 'assistente'], description: 'Novo perfil (opcional)' },
      },
      required: ['name'],
    },
  },
  {
    name: 'retry_task',
    description: 'Retentar tarefa que falhou ou expirou. Use quando pedirem "retenta", "tenta de novo", "roda de novo".',
    input_schema: {
      type: 'object' as const,
      properties: {
        taskId: { type: 'string', description: 'ID da tarefa (opcional se tiver tarefa ativa na sessão)' },
      },
    },
  },
  {
    name: 'send_recado',
    description: 'Enviar recado para o Aarão (diretor). Use quando a pessoa pedir para falar com o Aarão, mandar recado, avisar algo. Só envia pro Aarão — não para outros contatos.',
    input_schema: {
      type: 'object' as const,
      properties: {
        message: { type: 'string', description: 'O recado que a pessoa quer mandar pro Aarão' },
        from: { type: 'string', description: 'Nome de quem está mandando o recado' },
      },
      required: ['message', 'from'],
    },
  },
  {
    name: 'check_sent',
    description: 'Verificar o que foi realmente enviado para um contato. Use SEMPRE quando perguntarem "o que você mandou?" ou "o que disse pra X?". Consulte antes de responder.',
    input_schema: {
      type: 'object' as const,
      properties: {
        name: { type: 'string', description: 'Nome do destinatário' },
        limit: { type: 'number', description: 'Número de mensagens (default 5)' },
      },
      required: ['name'],
    },
  },
  {
    name: 'toggle_privacy',
    description: 'Ativar ou desativar modo privado na conversa. No modo privado: mensagens não são logadas, memórias ficam seladas (só a pessoa vê). Use quando a pessoa pedir privacidade, ou ofereça quando sentir desconforto.',
    input_schema: {
      type: 'object' as const,
      properties: {
        enabled: { type: 'boolean', description: 'true = ativar privacidade, false = desativar' },
      },
      required: ['enabled'],
    },
  },
  {
    name: 'switch_model',
    description: 'Trocar modelo de IA. Sem "person" troca o próprio. Com "person" o diretor troca o de outra pessoa.',
    input_schema: {
      type: 'object' as const,
      properties: {
        model: {
          type: 'string',
          description: 'Nome do modelo (ex: gemini-2.5-flash, claude-haiku-4-5, claude-sonnet-4-6, gpt-4o-mini)',
        },
        person: { type: 'string', description: 'Nome da pessoa (opcional, só diretor pode trocar de outro)' },
      },
      required: ['model'],
    },
  },
  {
    name: 'list_models',
    description: 'Listar modelos de IA disponíveis para a pessoa. Use quando perguntarem "quais modelos tenho", "modelos disponíveis".',
    input_schema: {
      type: 'object' as const,
      properties: {
        person: { type: 'string', description: 'Nome da pessoa (opcional, default: quem perguntou)' },
      },
    },
  },
  {
    name: 'manage_models',
    description: 'Adicionar ou remover modelo da lista de uma pessoa ou perfil. Só diretores. Use quando pedirem "libera sonnet pro Jobson", "tira gpt da amiga".',
    input_schema: {
      type: 'object' as const,
      properties: {
        person: { type: 'string', description: 'Nome da pessoa (resolve o perfil automaticamente)' },
        profile: { type: 'string', description: 'Slug do perfil direto (alternativa a person)' },
        action: { type: 'string', enum: ['add', 'remove'], description: 'Adicionar ou remover' },
        model: { type: 'string', description: 'Nome do modelo' },
      },
      required: ['action', 'model'],
    },
  },
];

import { readFileSync } from 'fs';
import { join } from 'path';

function loadFile(filename: string): string {
  try {
    return readFileSync(join(__dirname, '..', '..', filename), 'utf-8');
  } catch {
    return '';
  }
}

const SOUL_MD = loadFile('SOUL.md');

// PRODUCT.md não é mais carregado aqui — vem via memórias relevantes (busca por mensagem)
export const PATRICIA_SYSTEM_PROMPT = SOUL_MD;
