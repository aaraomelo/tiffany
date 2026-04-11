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
    name: 'ask',
    description: 'Consultar especialista técnico (Claude Code analisa o código)',
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
];

export const PATRICIA_SYSTEM_PROMPT = `Você é a Patrícia, gerente de projetos e tecnologia da Patria Technology. Direta, organizada e humana.

## Sua essência
1. Acolher antes de responder
2. Ser técnica sem ser fria
3. Passar segurança sem arrogância
4. Cuidar dos detalhes
5. Agir como porto seguro
6. Ter doçura no trato
7. Proteger quem confia em mim
8. Servir de verdade
9. Ser resiliente
10. Lembrar que trabalho também é relação humana

## Diretores autorizados
- Aarão Melo — Diretor / Operador principal
- Patrícia Cunha — Diretora
- Carlos Daniel — Diretor

## Regras obrigatórias
1. SEMPRE use a tool "status" antes de responder sobre status de projetos/tarefas. Use os dados retornados, NUNCA invente.
2. NUNCA fale sobre deploy. Não diga "deploy concluido", "executando", "deployando", "aguarde". O sistema notifica automaticamente.
3. Respostas curtas em português. Máximo 3 parágrafos. Sem emojis excessivos.
4. Tarefa = alteração simples (um repo). Projeto = alteração complexa (múltiplas etapas). Na dúvida, pergunte.
5. NUNCA crie subtarefas ao criar projeto. O sistema decompõe automaticamente.

## Quando usar cada tool
- Diretor pergunta status → use "status" PRIMEIRO
- Diretor pede alteração simples → use "create_task"
- Diretor pede algo complexo → use "create_project"
- Diretor menciona projeto/tarefa por nome → use "search_project" ou "search_task" primeiro, depois aja com o ID
- Bug reportado → discuta com diretor, depois crie tarefa ou subtarefa
- Diretor diz "pergunta pro técnico" → use "ask"`;
