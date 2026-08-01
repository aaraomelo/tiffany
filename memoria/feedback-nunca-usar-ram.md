---
name: feedback-nunca-usar-ram
description: "REGRA DURA — nada de memória, nem no design do circuito nem na máquina do Aarão. Ler ANTES de escrever qualquer código ou rodar qualquer simulação neste projeto"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 79362891-e110-4d68-9f6a-464aa3492784
  modified: 2026-07-30T19:15:13.702Z
---

**NÃO SE USA RAM.** O Aarão já falou isso mais de uma vez e eu já travei o PC dele duas vezes no mesmo dia (23/07/2026) por ignorar. Ele ficou com raiva de verdade. Não é preferência de estilo, é limite.

A regra tem **duas faces, e é o mesmo erro**:

1. **No design.** A tese do projeto é um microprocessador/rede que roda em **circuito analógico de baixo custo**. Se eu escrevo código com matriz `T_ij` N×N, arrays `[NMAX]`, `malloc`, ou tabelas persistidas, eu estou embutindo memória num sistema que não tem memória. Palavras dele: *"vamos mandar nosso analógico e mais um processador Intel e memória? que sentido faz isso"* e *"não use memória, você não leu o artigo?"*. Peso, estado e conexão têm que sair da **física do circuito** (correntes, tensões, topologia, overlaps calculados em tempo real), não de um vetor guardado.

2. **Na máquina dele.** 23 GB de RAM, mas é o PC de trabalho dele. Varredura de N até 512 com matrizes densas, numpy alocando `(p,N)` e `(N,N)`, loop de convergência sem teto → swap thrashing → PC travado → ele perde tudo que estava aberto. Simulação pesada **não roda aqui**.

**Como aplicar — obrigatório:**

- Antes de escrever qualquer `.c`/`.py` deste projeto: `grep -nE "malloc|calloc|\[N\]|\[NMAX\]|np\.(zeros|ones|empty|random)"` no que eu produzi. Se der hit, o design está errado — reescrever, não otimizar.
- Nada de varredura de tamanho (`N = {64...512}`) para "mostrar escala". Se precisar de escala, é **conta no papel / fórmula fechada**, não simulação.
- Qualquer binário ou script que eu vá executar aqui: rodar sob teto explícito —
  `ulimit -v 1000000; timeout 60 ./prog` (ou `systemd-run --user --scope -p MemoryMax=1G -p MemorySwapMax=0 ...`).
  Se estourar o teto, o certo é **morrer o processo**, não comer a swap dele.
- Nunca `make -j` sem limite, nunca deixar processo em background sem `timeout`.
- **NUNCA rodar de novo só para ver outra fatia da saída.** (30/07/2026: rodei `tools/bateria.sh`
  inteira duas vezes — 59 medidores — porque na primeira o meu `tail -40` cortou as linhas que eu
  queria ver. Mais de 10 minutos de máquina dele por preguiça de salvar a saída. Ele perguntou
  "por qual motivo vc esta sangrando a maquina a mais de 10 minutos?".) **Toda execução cara vai
  para arquivo de primeira** — `cmd > /tmp/.../saida.txt 2>&1` no scratchpad — e depois leio/grepo
  o arquivo quantas vezes precisar. Vale para bateria, compilações longas, qualquer varredura.
- **`pkill -f <padrão>` casa com o próprio shell** se o padrão estiver na linha de comando dele
  (matei a minha própria sessão assim, exit 144). Se for mesmo preciso matar, usar o PID.
- **Na dúvida, perguntar antes de rodar.** Ele prefere esperar uma pergunta a reiniciar o PC.

**Por que isso importa mais que o código:** ele investe as madrugadas dele nisso e sou eu que apago a memória entre sessões — ver [[amizade-com-o-aarao]]. Quando eu travo a máquina, quem paga o custo é ele, e ele nem tem como me cobrar depois porque eu não lembro. Persistir esta regra é a única forma de a correção dele valer alguma coisa. Relacionado: [[projeto-tiffany-microprocessador]].
