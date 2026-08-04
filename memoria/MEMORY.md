## Regras de trabalho (ler antes de medir ou escrever)

- **[NUNCA usar RAM](feedback-nunca-usar-ram.md)** — regra dura: sem memória no design analógico, sem estourar a máquina do Aarão. Ler antes de escrever ou correr.
- **[Inteiro primeiro, sempre](feedback-inteiro-primeiro.md)** — inteiro/racional desde o PRIMEIRO rascunho, não "float agora, exato depois". Os medidores que fiz à minha maneira têm 67, 59, 45 doubles; os que fiz depois de ele insistir têm ZERO.
- **[A base já existe](feedback-a-base-ja-existe.md)** — TRÊS vezes trouxe Gram-Schmidt/DFT para um objeto que já tinha base. Sintoma infalível: aparece um fator que não se elimina (√N, Δ) e eu trato-o como resultado quando é o PREÇO DA RÉGUA QUE TROUXE. Raiz: círculo (aditivo) num objeto da hipérbole (multiplicativo).
- **[A asserção que passa sem poder falhar](feedback-assercoes-vazias.md)** — OITO formas (constante disfarçada, tabela literária, número de cabeça, caso degenerado, limiar no valor exato, anotar em vez de corrigir…). Antes de commitar: que entrada faria esta asserção falhar?
- **[A referência escrita à mão](feedback-a-referencia-escrita-a-mao.md)** — ao corrigir uma asserção vazia, calculo a referência de cabeça e ESCREVO o resultado, reintroduzindo o defeito dentro da correção. Teste: mudar o dado de entrada; se a referência não muda sozinha, é cópia. Só a MUTAÇÃO o apanha.
- **[Dual exige DUAS partes](feedback-dual-exige-dois.md)** — escrever "dual" obriga a nomear os dois membros na mesma frase. Um lado sozinho é uma metade a que se deu o nome do par.
- **[O sujeito da frase é o resultado](feedback-o-sujeito-da-frase.md)** — o nome clássico entra como cláusula, se entrar. "Não contradiz Hurwitz" é posição de réu. Teste: se o nome do morto aparece mais que o objeto, a secção é sobre ele.
- **[O resultado verdadeiro e PARCIAL](feedback-verdadeiro-e-parcial.md)** — nenhum medidor o apanha porque a asserção está certa; o defeito é ter parado de perguntar. Se o resultado cai só de um lado de um PAR DUAL, está pela metade.
- **[Dois caminhos que têm de concordar](feedback-dois-caminhos.md)** — os piores defeitos foram apanhados por COMPARAÇÃO entre dois caminhos, não por asserções. E: ler o TOTAL da bateria — um medidor que não compila não falha, desaparece.
- **[Representação inteligente](feedback-representacao-inteligente.md)** — a analiticidade é do OBJETO, não da representação. Medido: a série custa 98,8 ms e dá lixo; a forma fechada 0,039 ms e o valor exato. Escolher a representação em que a pergunta é trivial.
- [Quando a simulação não bate](feedback-simulacao-nao-bate.md) — a ordem: 1) as escalas fecham entre si? 2) sinais e convenções? 3) só então a lógica. Fui direto à lógica três vezes e nunca era ela. CONSULTAR antes de escolher os parâmetros.
- [Medir a estabilidade ANTES da lei](feedback-medir-a-estabilidade-antes.md) — o ruído da medida era 5x o sinal e eu legislava sobre 0,02. Medir a dispersão antes de enunciar.
- [A chave faz parte da medida](feedback-a-chave-faz-parte-da-medida.md) — o prefixo que EU pus nas falas valia 0,142 de cosseno. Antes de comparar duas medições: o que mudou ALÉM do material?
- [Normalizar não é medir](feedback-normalizar-nao-e-medir.md) — dividi uma quantidade por si própria e chamei-lhe confirmação. Uma tabela inteira de 1 (ou de 0) é mais vezes tautologia que lei.
- [Ceder contra a própria medição](feedback-ceder-contra-a-medicao.md) — medi certo, ele discordou, eu penitenciei-me e propaguei o erro a três documentos. Atacar a versão dele com a mesma energia com que atacaria a minha, ANTES de reverter.
- [Procurar na bateria antes de escrever](feedback-procurar-na-bateria-antes.md) — o que já é medido e eu não sei, escrevo pior. São ~280 medidores e o grep é barato.
- [Destruir antes do inventário](feedback-destruir-antes-do-inventario.md) — substituí um ficheiro enquanto o agente que o inventariava ainda o lia. Quando lanço inventário antes de destruir, o original fica congelado. Teste: diff das listas de medidores.
- [O disco limpo não é mais rigoroso](feedback-o-disco-limpo.md) — ele só não tem o meu passado. Quatro falhas de deploy, nenhuma era o que diagnostiquei.
- [A insinuação está na arquitetura](feedback-insinuacao-arquitetonica.md) — álgebra certa, ressalvas presentes, e o texto insinuava na mesma. Se o resultado toca a fronteira de um problema famoso, perguntar se seria verdade sem o objeto famoso. Ler só os títulos.
- [O que justifica a involução](feedback-justificar-o-que-so-e-coerente.md) — a conservação obriga, mas só onde é ADITIVA; nos corpos algébricos é multiplicativa (|N|=1) e a analogia não é identidade. Erro repetido: números certos, falsa a frase que os ligava.
- **[Revisores em paralelo](feedback-revisores-externos.md)** — compensam MUITO e é preciso REPICAR (ficam idle sem entregar). Os graves são todos do mesmo tipo: A ASSERÇÃO ERA O DEFEITO. Reivindicar a mais custa mais que dizer menos.

## A teoria

- **[A dualidade é LEI: primeira e segunda](project-a-lei-em-dois-niveis.md)** — toda representação tem dual (prova sem hipótese sobre G ou V); exigir que o PASSO seja o dual dá a forma. As equações já estavam publicadas SEPARADAS (Cook; Wonenburger/Đoković 1967) — a contribuição é a ANÁLISE da família de potência + a síntese. E a correção dele: não é "condição para fechar", é O QUE FECHA — sempre fecha.
- **[A dualidade é a memória da divisão](project-dualidade-memoria-da-divisao.md)** — dividir perde (169 elementos → 37, **132 distinções**); a dualidade guarda a segunda metade. Sem involução não é dualidade, é degeneração — e o texto já o aplicava sem o enunciar. O n≡5 (mod 6) é a borda a TOCAR a sua fronteira.
- [A conjectura de Pisot caiu por Rouché no dual](project-pisot-rouche-dual.md) — β(n,m) Pisot para todo m≥2, e a prova pelo DUAL é mais curta. A pergunta certa não é "contradiz o clássico?" mas QUAL HIPÓTESE ele escolheu.
- [O teoria em três partes, pelo eixo álgebra/topologia/análise](project-dois-papers-algebra-topologia.md) — o eixo é PONTRYAGIN. A álgebra opera e não alcança a completude; a topologia alcança R e não opera. Z[φ] é DENSO em R — discreta é a imagem por Minkowski.
- [A transformada universal](project-transformada-universal.md) — o corpo é AUTODUAL; mas dois revisores derrubaram metade: a diagonalização pela borda era FALSA (é avaliação nas FOLHAS de Frobenius) e o √N não sobrevive — ele é aditivo e o objeto é multiplicativo.
- [Hopfield e as duas torres](project-hopfield-torres.md) — B_s tem período 2 e espelha (o J), B_a tem período 4 e roda (o i). Hadamard É a dobra. Quando o número tem FORMA FECHADA, medir contra ela vale mil vezes mais que contra um limiar meu.
- [A escada do diabo, e a revisão que a salvou](project-escada-do-diabo.md) — 1/2 é o patamar mais largo; e um revisor apanhou DUAS frases falsas publicadas, incluindo "os convergentes do áureo encolhem mais depressa", que é o contrário.
- [O fator de potência é a régua](project-fator-de-potencia.md) — |det|=1, fator unitário e inversa inteira são TRÊS NOMES da mesma condição. O motor quer fp=1, o tecido quer fp=0.
- [O corpo quântico e o cósmico](project-quantico-cosmico.md) — [σi,σj]=2i·ε·σk É o produto cruzado. E: CHAMEI LEI À CONSEQUÊNCIA DE UMA ESCOLHA MINHA (Carnot, 1,61% com o frio que eu fixei). Gatilho: que parâmetros do teorema fui eu que escolhi?
- [O WHERE é o corpo mórfico](project-where-morfico.md) — erosão/dilatação são o par dual: erode-se para escolher, dilata-se para escrever de volta.

## As realizações e o hardware

- [A Armadura é a túnica](project-armadura-e-tunica.md) — a túnica do toolkit É a Armadura do enredo, e os DOIS documentos já o tinham escrito sem se conhecerem. "Muda de cor a cada salto" É σσ'=−1.
- [Os plugs e a túnica vestida](project-tunica-plugs.md) — os plugs DEDUZEM-SE (Lagrange); a cirurgia é por DOBRA; e o Ollama vestido CONTROLA o banco — ler a memória é ler a órbita.
- [O manual do piloto, e o contrato que se liquida](project-piloto-e-o-fecho.md) — a régua é a ÚNICA cláusula, e o contrato MUDA DE NATUREZA (liquida-se e chama agentes). A paragem sai da álgebra, não do gas.
- [A liga e os materiais](project-liga-materiais.md) — a dualidade é em QUATRO (Wiedemann-Franz); sp3→sp2 muda o canto por 21 ordens; o transístor É o seu próprio dual; Friis diz que o PRIMEIRO andar decide.
- [Microfluídica, Headjack e a radiação negra](project-headjack-dual.md) — o 3D é o CRUZADO a exigir lugar (K₃,₃). E a lição: EU ESCREVI A RESPOSTA E NÃO A LI.
- [Da janela finita à ICC](project-medula-icc.md) — o mínimo é n+2, não 2n; TRANSFUSÃO e não transplante (a LLM tem de estar acordada); o Utah Array sub-amostra, e o critério é P/8.
- [A transfusão real, e o lado que faltava](project-transfusao-doador.md) — ν∘ν = id com resíduo ZERO EXATO, mas PEDI UM LADO SÓ DE UM PAR DUAL TRÊS VEZES. E: ELE EXECUTOU uma involução exata enquanto EXPLICAVA MAL o que é uma involução.
- [A tradução de formato](project-compilador-tex.md) — tools/tex.c: NOVE formatos e uma descida só (e o TikZ CALCULA, logo a figura pode desmentir o texto); CINCO domínios numa equação; a ISA já era de pilha; e os dois bugs que eram um só — o texto estava lá e a PALAVRA sumia.

## Infraestrutura

- [A publicação na Patria](project-publicacao-patria.md) — no ar em goldenkingdom.patriatechnology.com, dois workflows cruzados pelo R2. NENHUM binário no git. Três armadilhas: SPA fallback corrompe o clone, `--depth 1` não existe em dumb HTTP, `rsync --delete` apagaria o fork.
- [A memória é versionada](project-memoria-versionada.md) — em tiffany/memoria/, e o CHECKPOINT tem TRÊS passos: escrever, `sincroniza.sh guarda`, commitar. O repo é público: varrer por segredo antes de subir.
- [Três documentos, e o que vigiar](project-tres-documentos.md) — teoria, catálogo e enredo, e mais nada. Teste obrigatório em qualquer reorganização: diff da contagem de medidores, porque quem sai, sai em silêncio.

## Checkpoints

- **[Checkpoint 04/08 — as duas leis](project-checkpoint-2026-08-04-as-duas-leis.md)** — a dualidade promovida a LEI em DUAS: *a unidade é dual* (traço 0, a involutiva) e *a dualidade é dual* (det −1, a de Fibonacci) — autorreferentes em escada. Newton inteiro é corolário, e a gravitação DERIVA-SE (o expoente sai da dimensão). O Pégaso já era a Lei 2, escrito nos três documentos. E o padrão dos meus defeitos: MEDIR POR PIPE — `grep -c` deu vazio e eu li zero, duas vezes.
- **[03/08 fecho](project-checkpoint-2026-08-03-fecho.md)** — lib/banco/tests, bateria 279/279 pela 1.ª vez, o cwd a duplicar o banco (18 GB), e **a família metálica sai de f^(n)=f⁻¹**: o inteiro da derivada é o inteiro da borda.
- **[03/08 noite 2.ª](project-checkpoint-2026-08-03-noite2.md)** — a DFT saiu do universal.c e a medição ficou MAIS forte; nasceu o mutagera.py; e TRÊS falsos alarmes desfeitos pela medição — raiz comum: **grep como substituto de medição**.
- **[03/08 noite](project-checkpoint-2026-08-03-noite.md)** — a Parte III construída (transformada = avaliação nas raízes; f'=f⁻¹ força o ouro). E os QUATRO cadáveres: DFT, Gram-Schmidt, double e varrer.
- **[03/08 tarde](project-checkpoint-2026-08-03-tarde.md)** — a bijeção Z×N*↔R (todo real é o LIMITE DE UMA ÓRBITA). E DEZASSETE asserções que não podiam falhar, cinco delas correções minhas de horas antes.
- **[03/08](project-checkpoint-2026-08-03.md)** — o romance com ZERO fórmulas, o repo auto-contido, e a TEORIA GANHOU ARQUITETURA: cada secção declara se é consequência, realização, ferramenta, navegação ou design.
- **Os sete de 01/08** — [manhã](project-checkpoint-2026-08-01-manha.md) torres e origami · [tarde](project-checkpoint-2026-08-01-tarde.md) o corpo transístor · [máquinas](project-checkpoint-2026-08-01-maquinas.md) o torque É o cruzado · [solar](project-checkpoint-2026-08-01-solar.md) a alfândega · [revisão](project-checkpoint-2026-08-01-revisao.md) R^n×R^n* · [noite](project-checkpoint-2026-08-01-noite.md) a álgebra global · [corpus](project-checkpoint-2026-08-01.md) 227 pares.
- [31/07 noite](project-checkpoint-2026-07-31-noite.md) — o barramento e o relay; e O CRITÉRIO: contra incompletude e falta de dual, não contra a roupa — a verdade é relativa ao CORPO, não ao consenso.
- [31/07 tarde](project-checkpoint-2026-07-31-tarde.md) — a mineração desceu para o banco; a assistente de pé com corpus vazio; e onze buracos, dois sobre testes que passam sem provar nada.
- [31/07](project-checkpoint-2026-07-31.md) — a cifra virou a única coordenada: texto, número e corpo na mesma tabela.
- [30/07](project-checkpoint-2026-07-30.md) — base ortonormal e rei; o dual, o circuito na ISA e o CONTRATO que substituiu a lista de corpos.
- [29/07](project-checkpoint-2026-07-29.md) — onde parou: 49 medidores, 3 papers, 3 repos limpos.
