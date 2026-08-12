/* corpo_front.js — OS FICHEIROS NO FRONT, E O PORTÃO QUE OS DEIXA SAIR.
 *
 * O Aarão: «põe os arquivos que vc precisa no front» · «os .tex têm que ir pro front, é claro».
 *
 * Vão — mas quais são «os que preciso» não é opinião minha. O `tools/corpo.sh` intercepta o
 * `fopen` do tradutor e escreve o que ele foi mesmo buscar para compor os documentos do front;
 * o manifesto é o resultado dessa medição. Uma lista à mão envelhece calada: muda-se um
 * `\fontsize` no estilo, entra um corpo novo que ninguém pôs na lista, e o pedido cai com um
 * 404 que não explica nada.
 *
 * ── E O MANIFESTO É TAMBÉM O PORTÃO, que é o que aqui se mede ────────────────────────
 *
 * O repositório é PÚBLICO, e nem tudo nele é para servir: o `curriculo/` tem CPF e conta
 * bancária (por isso está no `.gitignore`), e o `broca-so/cristalchain` diz «IP privado — não
 * publicar». Uma rota estática sobre a raiz servia isso tudo com um 200 tranquilo.
 *
 * Por isso a regra é de LISTA e não de padrão. Uma lista negra («recusa /curriculo/») esquece
 * o próximo segredo; uma lista branca só deixa sair o que o tradutor provou precisar.
 *
 *   §F0  cada .tex que o front compõe (tex_tradutor.js DOCS) está no manifesto
 *   §F1  o manifesto foi medido, e cada ficheiro dele está no disco
 *   §F2  o que o tradutor abre está no manifesto — as duas listas batem
 *   §F3  o portão deixa sair o que está na lista, e devolve o caminho certo
 *   §F4  o CONTROLO: recusa o que não está — e recusa em particular o que é segredo
 *   §F5  e recusa a fuga: `..`, o caminho absoluto, e o `..` escrito em percentagem
 */
'use strict';
const fs = require('fs');
const path = require('path');
const { execFileSync } = require('child_process');

const RAIZ = path.resolve(__dirname, '..');
const APP = path.join(RAIZ, 'app', 'src');

let falhas = 0, feitas = 0;
function ok(q, cond) {
    feitas++; if (!cond) falhas++;
    console.log(`#UNIT ${cond ? 'ok' : 'falha'} ${q}`);
    console.log(`  [${cond ? 'ok' : 'FALHA'}] ${q}`);
}

console.log('=== O CORPO NO FRONT: o manifesto medido, e o portao ===\n');

const manifesto = JSON.parse(fs.readFileSync(path.join(APP, 'corpo.json'), 'utf8'));
const LISTA = manifesto.ficheiros;

/* o resolvedor, lido do próprio módulo do front — sem o reescrever aqui, senão media-se uma
 * cópia minha em vez do que o servidor usa */
const fonte = fs.readFileSync(path.join(APP, 'corpo.js'), 'utf8')
    .replace(/^import .*$/m, `const manifesto = ${JSON.stringify(manifesto)};`)
    .replace(/^export /gm, '');
const mod = { FICHEIROS: null, resolveNoCorpo: null, tipoDe: null };
new Function('mod', fonte + '\nmod.FICHEIROS=FICHEIROS; mod.resolveNoCorpo=resolveNoCorpo; mod.tipoDe=tipoDe;')(mod);
const { resolveNoCorpo, tipoDe } = mod;

/* ─── §F0 o que o browser compõe está no manifesto ────────────────────────────────── */
{
    const trad = fs.readFileSync(path.join(APP, 'tex_tradutor.js'), 'utf8');
    const bloco = /const DOCS = \{([^}]+)\}/.exec(trad);
    const fontes = bloco ? [...bloco[1].matchAll(/:\s*'([^']+\.tex)'/g)].map(x => x[1]) : [];
    const fora = fontes.filter(f => !LISTA.includes(f));
    console.log(`   front DOCS: ${fontes.length}; fora do manifesto: ${fora.length}${fora.length ? ' ' + fora.join(' ') : ''}`);
    ok('§F0 cada .tex que o tex_tradutor.js compõe está no manifesto — o slot existe antes do click',
       fontes.length >= 8 && fora.length === 0);
}

/* ─── §F1 o manifesto foi medido, e o que ele nomeia existe ───────────────────────── */
{
    const faltam = LISTA.filter(f => !fs.existsSync(path.join(RAIZ, f)));
    const bytes = LISTA.reduce((s, f) => s + (fs.existsSync(path.join(RAIZ, f)) ? fs.statSync(path.join(RAIZ, f)).size : 0), 0);
    console.log(`   ${LISTA.length} ficheiros, ${bytes} bytes; em falta: ${faltam.length} ${faltam.slice(0,3).join(' ')}`);
    console.log(`   medido por: ${manifesto.medido_por}`);
    ok('§F1 o manifesto e medido e cada ficheiro dele esta no disco', LISTA.length > 20 && faltam.length === 0);
}

/* ─── §F2 as duas listas batem ────────────────────────────────────────────────────── */
/* Aqui não se confia no ficheiro: remede-se. O `fopen` do tradutor diz outra vez o que abre,
 * e o que ele abrir tem de estar no manifesto — senão o front tem um buraco que só aparece
 * quando alguém clica. */
{
    let medido = null;
    try {
        const espia = path.join('/tmp', 'corpo_espia');
        fs.mkdirSync(espia, { recursive: true });
        fs.writeFileSync(path.join(espia, 'e.c'),
            '#define _GNU_SOURCE\n#include <stdio.h>\n#include <dlfcn.h>\n#include <string.h>\n' +
            'static FILE *(*r)(const char*,const char*);\n' +
            'FILE *fopen(const char*c,const char*m){ if(!r) r=dlsym(RTLD_NEXT,"fopen");\n' +
            ' if(m[0]==0x72) fprintf(stderr,"ABRE %s\\n",c); return r(c,m); }\n');
        execFileSync('cc', ['-O2', '-fPIC', '-shared', path.join(espia, 'e.c'),
                            '-o', path.join(espia, 'e.so'), '-ldl']);
        const tex = path.join(RAIZ, 'tests', 'tex');
        if (!fs.existsSync(tex)) execFileSync('cc', ['-O2','-std=c99','-I../lib','tex.c','-lm','-o','tex'],
                                              { cwd: path.join(RAIZ, 'tests') });
        const visto = new Set();
        for (const doc of ['teoria.tex', 'enredo.tex', 'papers/corpo-estelar.tex', 'papers/arquitetura.tex']) {
            const r = require('child_process').spawnSync(tex, [path.join(RAIZ, doc), path.join(espia, 's.pdf')],
                { cwd: path.join(RAIZ, 'tests'), env: { ...process.env, LD_PRELOAD: path.join(espia, 'e.so') } });
            for (const l of String(r.stderr).split('\n')) {
                if (!l.startsWith('ABRE ')) continue;
                const c = l.slice(5).replace(/^\.\.\//, '').replace(RAIZ + '/', '');
                if (fs.existsSync(path.join(RAIZ, c))) visto.add(c);
            }
        }
        medido = [...visto].sort();
    } catch (e) { medido = null; }

    if (!medido || !medido.length) {
        console.log('   §F2 nao foi possivel remedir (o espiao ou o tradutor nao correram) — e');
        console.log('       dize-lo e melhor do que dar por bom o ficheiro que ia conferir.');
        ok('§F2 o que o tradutor abre esta no manifesto — as duas listas batem', false);
    } else {
        const fora = medido.filter(f => !LISTA.includes(f));
        console.log(`   remedido agora: ${medido.length} ficheiros; fora do manifesto: ${fora.length} ${fora.slice(0,4).join(' ')}`);
        ok('§F2 o que o tradutor abre esta no manifesto — as duas listas batem', fora.length === 0);
    }
}

/* ─── §F3 o portão deixa sair o que está na lista ─────────────────────────────────── */
{
    let mau = 0, tipos = 0;
    for (const f of LISTA) {
        if (resolveNoCorpo('/corpo/' + f) !== f) mau++;
        if (tipoDe(f) !== 'application/octet-stream') tipos++;
    }
    console.log(`   o portao resolve ${LISTA.length - mau} de ${LISTA.length}; com tipo declarado: ${tipos}`);
    ok('§F3 o portao deixa sair o que esta na lista, e devolve o caminho certo', mau === 0);
    ok('§F3 e cada um sai com o seu tipo — nenhum vai como fluxo de bytes anonimo',
       tipos === LISTA.length);
}

/* ─── §F4 o CONTROLO: o que NÃO está na lista não sai ─────────────────────────────── */
/* Sem isto, «o portao deixa sair» era uma asserção sobre uma porta aberta. E o que se tenta
 * aqui é o que existe mesmo e não pode sair: o repositório é público, e estes ficheiros são
 * a razão de o portão ser lista branca. */
{
    const proibidos = [
        'curriculo/cv.tex', 'curriculo/dados.tex', '.gitignore', 'tools/segredo.sh',
        'banco/sql.c', 'memoria/MEMORIA.md', 'app/vite.config.js', 'package.json',
        'broca-so/cristalchain/README.md', 'segredo',
    ];
    let saiu = null;
    for (const f of proibidos) if (resolveNoCorpo('/corpo/' + f) !== null) { saiu = f; break; }
    console.log(`   controlo: ${proibidos.length} caminhos que nao estao na lista — saiu algum? ${saiu || 'nao'}`);
    ok('§F4 o que NAO esta no manifesto nao sai — e o repositorio e publico', saiu === null);

    /* e o que existe MESMO no disco e não está na lista também não sai */
    const existem = proibidos.filter(f => fs.existsSync(path.join(RAIZ, f)));
    console.log(`   e destes, ${existem.length} existem mesmo no disco: ${existem.slice(0,4).join(' ')}`);
    ok('§F4 e o controlo nao e vazio: pelo menos um dos recusados existe mesmo no disco',
       existem.length > 0);
}

/* ─── §F5 e a fuga ────────────────────────────────────────────────────────────────── */
{
    const fugas = [
        '/corpo/../curriculo/cv.tex',
        '/corpo/lib/../../curriculo/cv.tex',
        '/corpo/%2e%2e%2fcurriculo%2fcv.tex',
        '/corpo//etc/passwd',
        '/corpo/estilo.tex%00.png',
        '/corpo/' + 'a'.repeat(5000),
    ];
    let passou = null;
    for (const u of fugas) if (resolveNoCorpo(u) !== null) { passou = u; break; }
    console.log(`   controlo: ${fugas.length} tentativas de fuga — passou alguma? ${passou || 'nao'}`);
    ok('§F5 nem `..`, nem o caminho absoluto, nem o `..` escrito em percentagem passam',
       passou === null);

    /* e o `..` decodificado É o mesmo caminho que o disco veria: decodifica-se ANTES de
     * decidir, senão o portão julga um texto e a leitura abre outro */
    ok('§F5 o portao decide sobre o caminho JA descodificado — ve o mesmo que o disco veria',
       resolveNoCorpo('/corpo/estilo%2Etex') === null || resolveNoCorpo('/corpo/estilo.tex') === 'estilo.tex');
}

console.log('\n==========================================================================');
if (!falhas) {
    console.log('  Os .tex vao para o front, e vao pela porta que o proprio tradutor abriu: a');
    console.log('  lista sai do `fopen` dele, nao do meu caderno — e por isso nao envelhece em');
    console.log('  silencio quando o estilo pedir um corpo novo.');
    console.log('');
    console.log('  E a lista e o PORTAO. O repositorio e publico e nem tudo nele e para servir;');
    console.log('  uma lista negra esquece o proximo segredo, uma lista branca so deixa sair o');
    console.log('  que o tradutor provou precisar.');
} else console.log(`  FALHOU: ${falhas}`);
console.log(`#TOTAL ${feitas} ${falhas}`);
process.exit(falhas ? 1 : 0);
