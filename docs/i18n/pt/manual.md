# Manual do usuário do {{APP}}

Este manual foi traduzido por máquina e ainda não foi revisado por um falante de português do Brasil. Onde algo não estiver claro, o manual em inglês é a referência.

O {{APP}} é um estúdio de escrita para textos longos: romances, roteiros de cinema, peças de teatro, roteiros de quadrinhos, teses e livros de não ficção. Você escreve em pequenos trechos (cenas, seções, notas), organiza-os com liberdade e, no fim, *compila* tudo em um único manuscrito.

Este manual trata da versão {{VERSION}}. Pressione **F1** a qualquer momento para abri-lo; ele abre na parte que corresponde ao que você está vendo: o documento, o quadro de cortiça, a estrutura de tópicos ou o Texto combinado, o Fichário, a aba do inspetor que está com o teclado ou a caixa de diálogo aberta (uma caixa de diálogo que não tem parte própria abre o início). A janela tem uma **lista de conteúdo** à esquerda (um clique em um título leva até ele) e um **campo de busca** acima do texto: pressione **Ctrl+F** para ir até ele, digite uma palavra e **Return** mostra a próxima ocorrência, **Shift+Return** a anterior (os botões **Próximo** e **Anterior** fazem o mesmo); a linha ao lado do campo diz qual ocorrência de quantas está sendo exibida, e todas as ocorrências ficam marcadas no texto. O que se destina a quem desenvolve o {{APP}} (plugins, pacotes, a pasta do projeto em detalhe, variáveis de ambiente) está no guia do desenvolvedor, **Ajuda > Solução de problemas e desenvolvimento > Guia do desenvolvedor: atualizações e complementos**.

---

## 1. Primeiros passos

### Como obter o programa

O {{APP}} é baixado na página **Releases** da página do projeto, https://github.com/Shelin-boop/Quillbinder_Collection/releases. Cada versão publicada tem o programa como um único arquivo (*{{APP}}-&lt;version&gt;-x86_64.AppImage*), uma atualização do programa para quem já tem um AppImage do {{APP}} (seção 17) e os complementos (arquivos *.qbpkg*, seção 17). Torne o AppImage executável (clique com o botão direito > **Propriedades** > **Permissões** > *É executável*, ou `chmod +x` em um terminal) e dê um clique duplo nele; **Ajuda > Adicionar ao menu de aplicativos** o coloca no menu da sua área de trabalho. O arquivo *SHA256SUMS.txt* de cada versão lista as somas de verificação dos seus arquivos.

**O {{APP}} é um software {{LABEL}}.** Mantenha backups dos seus projetos (seção 15) e uma cópia dos seus manuscritos fora do programa.

### Como iniciar um projeto

Quando o {{APP}} inicia sem um projeto, a **janela inicial** é aberta:

- **Novo projeto**: escolha uma categoria e um modelo (por exemplo Em branco, Romance, Romance com partes ou Conto), digite um nome, escolha onde salvar e clique em **Criar**. **Formato** define como o projeto é armazenado: como um projeto do {{APP}} (o padrão) ou como um projeto do Scrivener 3 (*.scriv*), para que você também possa abri-lo no Scrivener (seção 14, *Como trabalhar no formato do Scrivener*).
- **Projetos recentes**: dê um clique duplo em um projeto em que você trabalhou antes.
- **Abrir outro…**: encontre um projeto no seu disco.
- **Importar projeto do Scrivener…**: crie um projeto do {{APP}} a partir de um projeto do Scrivener (veja a seção 14). Para continuar trabalhando no próprio projeto do Scrivener, abra o arquivo *.scrivx* dele com **Abrir outro…**.

O {{APP}} reabre o último projeto ao iniciar (você pode desativar isso em **Arquivo > Opções > Geral**).

**A primeira inicialização** mostra uma única página com tudo já preenchido (**OK** mantém os valores): os backups (se um projeto passa por backup ao ser fechado, quantos backups manter, 25 no início, e onde), a partir de um AppImage do Linux a pergunta se o {{APP}} deve ser adicionado ao menu de aplicativos (e se o AppImage deve ser movido para *~/Applications*) e o aviso sobre a verificação de atualizações com o respectivo botão para ativar ou desativar. O projeto de boas-vindas é aberto quando você pressiona **OK**; nenhuma janela vazia do programa aparece antes dele. Tudo isso pode ser alterado depois em **Arquivo > Opções**; lá, em **Geral > Inicialização**, também há a escolha de quais configurações têm prioridade: as do próprio projeto ou sempre as do programa (seção 16, *Configurações de um projeto*).

### O projeto de boas-vindas e o Guia de início rápido

Na primeira vez que o {{APP}} inicia, ele abre o **projeto de boas-vindas**: um projeto curto que mostra, um documento por assunto, o que o {{APP}} tem além do usual (a barra de ícones, um zoom para cada parte da janela, mapas mentais, fichas de objetos, os comentários de uma pasta inteira, Comparar e transferir, projetos do Scrivener, temas, atualizações e complementos, e o que protege o seu texto). Comece pelo documento **Boas-vindas** no alto do Fichário; os links dele levam a cada parada do passeio. Nada do que você fizer no projeto de boas-vindas pode prejudicar os seus próprios projetos. (Um projeto passado na linha de comando, ou um arquivo de projeto aberto com clique duplo, abre no lugar dele; o projeto de boas-vindas aparece então na próxima inicialização.)

O projeto de boas-vindas é salvo como "Welcome to {{APP}}" na sua pasta *Documentos*. **Ajuda > Projeto de boas-vindas…** o abre novamente como você o deixou, ou cria uma cópia nova para recomeçar (**Criar uma cópia nova**; a cópia antiga vai para a lixeira da área de trabalho). Exclua-o quando quiser; o comando cria um novo.

**Ajuda > Guia de início rápido** mostra em uma única página onde ficam as configurações importantes (temas e modo escuro, cores e fontes, formatação, zoom, barras de ferramentas, atalhos, backups ...) e resume o que o {{APP}} sabe fazer.

### Como salvar

Você nunca precisa salvar manualmente. O {{APP}} salva automaticamente alguns segundos depois de você parar de digitar, quando você troca para outro programa e quando fecha o projeto. Se você continua digitando, ele salva na primeira pausa curta (uma respirada entre palavras) assim que passam 5 segundos, e no máximo após 10 segundos, mesmo sem nenhuma pausa. **Ctrl+S** salva imediatamente, se você quiser ter certeza.

Cada arquivo é gravado primeiro com um nome temporário e só substitui o antigo quando foi gravado por inteiro e o computador o colocou no disco. Se o disco ficar cheio ou a unidade falhar durante o salvamento, o arquivo antigo permanece como estava e as suas alterações continuam na janela até que o salvamento volte a funcionar.

**O estado do salvamento está sempre na tela** (0.70). A barra de rodapé do primeiro editor (o trecho vazio entre o total da pasta e a contagem de palavras, ou à esquerda nas outras barras de rodapé) diz **Salvo**, **Ainda não salvo** (alguns segundos depois de você digitar) ou, sobre fundo vermelho, **Não salvo: o último salvamento falhou às 14:03**; enquanto os salvamentos falham, o título da janela também traz uma marca. Um salvamento que falhou também mostra uma mensagem que nomeia os documentos não salvos e oferece **Salvar textos em outro lugar…**; ela nunca toma o teclado, de modo que a próxima tecla que você digitar ainda vai para o seu texto. O programa tenta novamente por conta própria (a cada 15 segundos no início, depois a cada minuto), a mensagem volta depois de cinco minutos de falhas, e o sinal vermelho permanece até que um salvamento funcione. Os mapas mentais são salvos e verificados da mesma forma: um quadro que não pode ser salvo é mantido na janela, contado como não salvo quando você fecha o projeto e entra na cópia de emergência.

**Mensagens curtas** (0.93.0). Não há uma linha de barra de status na parte de baixo da janela. As mensagens que antes apareciam ali ("Saved again: all changes are on the disk.", o resultado de um backup e semelhantes) aparecem pelos seus segundos no mesmo lugar do texto de salvamento, na barra de rodapé do primeiro editor (o da esquerda ou o de cima, quando há dois), e o texto de salvamento volta depois delas. Uma mensagem longa demais para o espaço é abreviada; pare o ponteiro sobre ela para ler tudo. Em um editor muito estreito, o texto é substituído por uma pequena marca colorida (verde salvo, amarelo ainda não salvo, vermelho falhou) com o texto como dica. No modo de composição, e enquanto nenhum projeto está aberto, nada é mostrado.

**Arquivos sobre os quais o {{APP}} não grava** (0.70). Um arquivo de texto que está danificado (um arquivo RTF cujo início está quebrado ou que está cortado) ou que outro programa salvou em outra codificação de caracteres (um arquivo Markdown salvo como Windows-1252 ou UTF-16, por exemplo depois de editá-lo fora do programa) é mostrado até onde pode ser lido e não pode ser editado; uma mensagem avisa isso, e o arquivo nunca é alterado. Copie o texto dele para um novo documento para continuar, ou salve o arquivo em UTF-8 no programa que o gravou e depois feche e reabra o projeto; ou volte a um dos seus instantâneos (desde 0.81.1): o arquivo danificado é antes mantido como cópia em *settings/conflicts* na pasta do projeto, e o texto do instantâneo passa a ser o texto do documento, que pode ser editado e é salvo novamente. Mesclar documentos e Dividir recusam um documento assim, informando o motivo, e Substituir no projeto, Converter aspas e as alterações de estilo o deixam como está e o nomeiam. Se um comando o alterar mesmo assim, o texto alterado não se perde: ele é mantido como um arquivo RTF em *settings/conflicts*, e uma mensagem diz onde (0.81.1). Um arquivo de mapa mental que não pode ser lido é tratado da mesma forma. Os caracteres que não podem ser armazenados (caracteres de controle de dados colados) são omitidos como antes; uma mensagem curta na barra de rodapé agora avisa isso uma vez por documento.

**Um salvamento chega por inteiro** (0.65). Quando um salvamento grava vários arquivos (o texto de um documento novo e o arquivo do projeto, um texto e suas imagens, a ficha de Objetos), todos eles são primeiro gravados ao lado dos seus lugares, depois listados em um pequeno diário (*settings/journal.json*) e só então colocados no lugar juntos. Se o programa ou o computador parar no meio, a próxima abertura conclui esse salvamento a partir do diário ou, se o diário ainda não estava completo, deixa o projeto como estava no salvamento anterior; um aviso diz qual dos dois. Um diário que não pode ser lido (uma falha de disco) nunca é tomado por um diário vazio: ele e os arquivos do seu salvamento são mantidos em uma pasta em *settings/conflicts*, nada é colocado no lugar nem excluído, e o aviso diz isso (0.81.1). Em uma pasta sincronizada, o diário de um salvamento do outro computador é deixado em paz por 15 minutos (aquele salvamento pode ainda estar em andamento lá, ou seus arquivos ainda estar chegando): este computador não salva o projeto nesse meio-tempo e tenta de novo por conta própria, e se o outro computador parou durante aquele salvamento, este o conclui depois desses 15 minutos (0.81.1). Se um arquivo de um salvamento não puder ser gravado, nenhum deles substitui coisa alguma, e as suas alterações continuam na janela para a próxima tentativa. Desde 0.93.0, uma unidade que informa um erro quando uma pasta é sincronizada depois de os arquivos terem sido colocados no lugar (um erro de verdade, não um sistema de arquivos incapaz de sincronizar) faz o salvamento contar como falho: o diário permanece, a mensagem diz que a unidade informou um erro, e o próximo salvamento ou a próxima abertura conclui o salvamento a partir do diário. Enquanto um diário que não pode ser lido não puder ser movido para o lado (a pasta *settings/conflicts* não pode ser gravada), nenhum salvamento grava um diário novo por cima dele: o salvamento espera, a sua mensagem nomeia o arquivo, e o programa salva por conta própria assim que o arquivo puder ser posto de lado. Um arquivo temporário de um salvamento anterior que esteja no caminho é movido para o lado, nunca sobrescrito. Uma sincronização de pasta que falha antes de qualquer coisa ser substituída é uma falha limpa: nada foi alterado. Quando o erro vem depois de os arquivos estarem no lugar, o programa anota o que está no disco (para que as novas tentativas não relatem falsos conflitos de "alterado em outro lugar" e não gravem imagens duas vezes) e sincroniza a pasta de novo a cada salvamento: "Salvo" sempre segue uma sincronização de pasta que foi verificada. Um sistema de arquivos que responde a uma sincronização de pasta com um erro já na abertura do projeto é lembrado como um que não pode sincronizar pastas, e os seus salvamentos não são informados como falhos.

Se você encerrar a sessão ou desligar o computador com o {{APP}} aberto, ele salva tudo antes de terminar (nenhum backup é feito nessa hora). Se esse salvamento falhar (disco cheio, uma unidade que sumiu), os textos e as notas que não foram salvos (e, desde 0.93.0, o arquivo do projeto como está na memória) são gravados como arquivos RTF em uma pasta de emergência, *~/.local/share/{{APP}}/Emergency*, e a próxima inicialização diz onde eles estão e quantos arquivos foram gravados (o aviso diz "incompleto" e nomeia o que falta quando um arquivo não pôde ser gravado). Se ele terminar inesperadamente, a próxima inicialização avisa (seção 20, *Relatórios de falhas*).

**Fechar um projeto que não pode ser salvo** pergunta o que fazer; **Manter aberto** é o padrão. **Salvar textos em outro lugar…** grava os textos e as notas ainda não salvos como arquivos RTF em uma pasta que você escolher (outra unidade, um pen drive), e o projeto continua aberto. Desde 0.93.0, essa cópia de emergência também contém o arquivo do projeto como está na janela (o Fichário, os títulos e as etiquetas), os quadros de mapa mental e os objetos alterados, e é feita também quando só o Fichário ou os Objetos foram alterados. A mensagem diz quantos arquivos foram gravados, como "n de m", e nomeia os arquivos que faltam e por quê; uma cópia à qual faltam arquivos recebe o título **incompleta** (com um sinal de aviso) e nunca se parece com uma cópia concluída. Uma pasta que já existe nunca é usada de novo: uma segunda cópia no mesmo segundo recebe um número. O arquivo do projeto na cópia é nomeado como o arquivo do projeto com ".copy" acrescentado (para que a pasta não seja tomada por um projeto por engano); o README na cópia diz como recolocá-la: feche o projeto, renomeie o arquivo do projeto na pasta do projeto (ele fica como estava), copie para lá o arquivo da cópia de emergência, renomeie-o com o nome do arquivo do projeto e abra o projeto de novo. Para um projeto no formato do Scrivener, o README diz que o arquivo pertence à cópia de trabalho e não dá caminho de volta. Quando, afinal, nada estava sem salvar (uma nova tentativa funcionou nesse meio-tempo), a mensagem diz que tudo está salvo e que nenhuma cópia era necessária. **Fechar sem salvar** fecha o projeto e perde essas alterações.

### Onde fica o seu trabalho

Um projeto é uma pasta chamada *SeuTítulo*{{SUFFIX}}. Dentro dela, cada documento é um arquivo de texto comum, de modo que a sua escrita continua legível mesmo sem o {{APP}}. Dê um clique duplo no arquivo *SeuTítulo*{{FILE}} dentro da pasta para abrir o projeto. Não renomeie nem mova arquivos dentro da pasta do projeto manualmente.

Para encontrar o arquivo de um documento fora do {{APP}}, abra *contents-by-title.txt* na pasta do projeto (0.65): ele lista cada documento do Fichário com o seu título, recuado conforme o seu lugar, e os arquivos que guardam o seu texto, as suas notas ou a sua mídia (por exemplo *content/<id>/text.md*). O {{APP}} o grava a cada salvamento que altera o Fichário e nunca o lê por conta própria. (Um projeto no formato do Scrivener não tem uma lista assim; o *.scrivx* do Scrivener nomeia os seus documentos.)

Um projeto deve estar aberto em apenas uma janela por vez. Se ele já estiver aberto em outra janela ou em outro computador (por exemplo por meio de uma pasta sincronizada), o {{APP}} avisa você antes de abri-lo. Iniciar o {{APP}} de novo com um projeto que está aberto em uma janela do programa (pelo gerenciador de arquivos ou por um atalho da área de trabalho) traz essa janela para a frente em vez de abrir o projeto duas vezes; outro projeto abre em uma janela própria.

**Um projeto em uma pasta sincronizada** (OneDrive, Dropbox, Nextcloud, Syncthing ou qualquer outro programa de sincronização) pode ser usado em vários computadores, um de cada vez. Feche-o em um computador e deixe o programa de sincronização terminar antes de abri-lo no outro. O {{APP}} ajuda nisso no nível dos arquivos, seja qual for o programa de sincronização:

- Enquanto um projeto está aberto, um pequeno arquivo de bloqueio nele nomeia o computador e é renovado a cada poucos minutos. Ele contém o nome do computador, o seu nome de usuário nele, a versão e o número de processo do programa, quando o projeto foi aberto e quando o bloqueio foi renovado pela última vez, e um código que distingue os computadores (um hash, não legível como nome). Qualquer pessoa com acesso à pasta compartilhada pode ver esses dados. Em **Arquivo > Opções > Geral > Salvamento**, **Nome deste computador em pastas compartilhadas** define um nome seu ("Notebook"); ele é usado no lugar do nome do computador nos arquivos de bloqueio, nos nomes das versões mantidas e em *settings/ui-<computer>.json*, e o seu nome de usuário é então omitido. Em outro computador, abrir o projeto mostra então o nome daquele computador, desde quando ele está com o projeto aberto e o seu último sinal de vida; **Abrir mesmo assim** ou **Cancelar**. Um bloqueio que não é renovado há mais de 15 minutos (o projeto não foi fechado lá, ou aquele computador está desligado) é mostrado como sobra, e **Abrir** é o padrão.
- Se o projeto for aberto em outro computador enquanto está aberto aqui, um aviso informa isso e oferece fechá-lo aqui.
- Se um arquivo do projeto foi alterado em outro lugar (em outro computador pelo programa de sincronização, ou por outro programa) depois de o {{APP}} lê-lo, e ele também foi alterado aqui, a outra versão é mantida antes de a sua ser salva: como **instantâneo** do documento (compare na aba Instantâneos do inspetor) e como cópia na pasta *settings/conflicts* do projeto. Um aviso diz quais documentos. Se a outra versão não puder ser mantida (por exemplo porque o disco está cheio ou *settings/conflicts* não pode ser gravada), a sua não é salva por cima dela: o salvamento falha com uma mensagem que nomeia o arquivo, e as suas alterações permanecem sem salvar até que a outra versão possa ser mantida. Um documento que você apenas olhou aqui nunca é salvo por cima de uma versão mais nova vinda de outro lugar.
- Abrir e fechar um projeto sem alterar nada não altera nenhum dos seus arquivos; o layout da janela e os lugares em que você estava em cada documento são mantidos por computador (*settings/ui-<computer>.json*). No formato do Scrivener, o *.scriv* também não é tocado, e ele recebe um arquivo de bloqueio do {{APP}} (*quillbinder.lock*) enquanto está aberto; o Scrivener não o lê. Antes de cada salvamento no *.scriv*, o {{APP}} procura o bloqueio do próprio Scrivener: se o Scrivener abriu o projeto depois do {{APP}}, nada mais é gravado no *.scriv* (um aviso diz isso uma vez, depois uma mensagem curta na barra de rodapé); as suas alterações permanecem na cópia de trabalho do {{APP}}. Feche o projeto no Scrivener e salve de novo (**Arquivo > Salvar**). Se você fechar o projeto aqui primeiro, as alterações vão para o *.scriv* na próxima vez que você o abrir aqui; onde o Scrivener alterou algo nesse meio-tempo, a versão dele é lida.
- Se o projeto lista um texto para um documento cujo arquivo ainda não chegou a este computador (o programa de sincronização não terminou), um aviso o nomeia depois da abertura; esse documento é mostrado vazio, não pode ser editado, e nada é salvo no lugar dele. Feche o projeto, espere até o programa de sincronização terminar e abra-o de novo. O mesmo vale para um texto que só pôde ser lido em parte (0.65).
- Um arquivo alterado em outro lugar também é percebido quando manteve o seu tamanho e a sua hora (alguns programas de sincronização definem a hora por conta própria): o {{APP}} compara o conteúdo dele com o que leu ou gravou (0.65).
- O conteúdo decide também no outro sentido (0.92.0): um arquivo cuja **hora apenas** mudou não é outra versão. Alguns programas de sincronização definem de novo a hora de um arquivo alguns segundos depois de o {{APP}} gravá-lo, quando o envio deles termina; nada é mantido por isso, nenhum aviso aparece, e o arquivo não é gravado de novo. Um arquivo de projeto que já contém o que um salvamento gravaria também é deixado em paz, de modo que um salvamento que não altera nada não dá ao programa de sincronização nada para transportar.
- O aviso **Outras versões mantidas** tem o botão **Não mostrar de novo até o {{APP}} ser fechado**. As outras versões continuam sendo mantidas como instantâneos e como cópias em *settings/conflicts*; só o aviso fica de fora até você sair do programa. A próxima inicialização o mostra de novo.
- **Um cadeado no que outro dispositivo está trabalhando (0.92.0).** Enquanto um projeto está aberto, cada dispositivo também anota, ao lado do arquivo de bloqueio, quais documentos ele tem abertos em um editor ou alterou desde que abriu o projeto. Quando um segundo dispositivo abre o projeto, o seu Fichário mostra um pequeno cadeado na ponta direita das linhas desses documentos, e nas pastas ao redor deles. Aponte para o cadeado: a dica nomeia o dispositivo e quando ele abriu o projeto. É uma sugestão, não uma proteção: nada é bloqueado, e você ainda pode editar o documento. A marca viaja pelo seu programa de sincronização, de modo que pode ter alguns minutos de atraso; ela exige a versão 0.92.0 ou mais nova nos dois dispositivos (o Scrivener não anuncia nada). Uma lista cujo dispositivo não deu sinal de vida há mais de 15 minutos não é mostrada. A anotação é um pequeno arquivo por dispositivo (*settings/lock-work-…json*; para um projeto no formato do Scrivener, *quillbinder.lock-work-…json* no *.scriv*); ele contém o nome do dispositivo como no arquivo de bloqueio, a hora e os números internos dos documentos, nenhum título e nenhum texto. Ele só é gravado quando a lista muda, nunca a cada tecla, é removido quando o projeto é fechado e fica de fora dos backups.
- **Formato do Scrivener, dois computadores** (0.65): documentos adicionados ao *.scriv* em outro lugar (o Scrivener em outro computador) enquanto o projeto está aberto aqui são mantidos, e o Fichário que os nomeia não é substituído; um aviso pede que você feche o projeto e o abra de novo, o que mostra tanto os documentos de lá quanto os seus. Imagens, PDFs, arquivos de áudio e vídeo e mapas mentais alterados no *.scriv* em outro lugar são mantidos como estão (antes, a versão daqui era copiada por cima deles); quando um foi alterado nos dois lados, a outra versão é mantida primeiro em *settings/conflicts*.

Um projeto no **formato do Scrivener** é, em vez disso, uma pasta *SeuTítulo.scriv*; abra-o com o arquivo *.scrivx* dele. O {{APP}} salva as suas alterações nele, de modo que o projeto continua sendo um projeto do Scrivener. **Arquivo > Salvar como…** salva um projeto com outro nome ou em outro lugar, em qualquer um dos formatos (seção 14).

### Privacidade: o que sai do seu computador

O {{APP}} acessa a internet por um único motivo: procurar atualizações. Ele busca um pequeno arquivo, não envia nada sobre você, e você pode desativar isso no menu Ajuda. (A seção 17, *Verificação de atualizações*, diz exatamente o que é buscado e quando. Antes de 0.92.0, o programa não procurava atualizações.) Mais três coisas passam pela internet, e somente quando você as inicia:

- **Arquivo > Importar > Página da web…** busca a página que você nomeia e as suas imagens nesse site. Como um navegador, ele envia o nome de um navegador comum (Firefox no Linux), os idiomas do seu sistema (para o idioma da página) e, como toda conexão, o endereço de internet do seu computador.
- **Baixar um modelo de idioma** para o complemento Assistente de tradução (seção 10) o busca nas versões publicadas do repositório do {{APP}} no GitHub. Ele envia o nome do complemento ("Quillbinder-Translation-Addon") e, como toda conexão, o seu endereço de internet.
- **Baixar um modelo de fala** (o complemento Transcrição de fala, seção 10) busca o modelo no Hugging Face (huggingface.co, uma empresa nos EUA) ou no endereço que você definir nas opções dele. Ele envia "{{APP}}/<version>" como nome do programa e, como toda conexão, o seu endereço de internet. Um modelo só é instalado quando a sua soma de verificação é a conhecida; um arquivo de outro endereço ou que você mesmo baixou só é usado depois que você concorda.

O que outras pessoas podem ver do seu trabalho:

- **Projetos em pastas compartilhadas** carregam um arquivo de bloqueio enquanto estão abertos (veja acima): nome do computador, nome de usuário e horários. Defina um nome seu em **Arquivo > Opções > Geral > Salvamento** para omitir esses dados.
- **Informações do autor** (nome, endereço, telefone, e-mail em **Arquivo > Opções**) ficam nas configurações do programa neste computador; elas nunca vão para as configurações do próprio projeto. Elas chegam a um manuscrito somente onde o formato de compilação usa os espaços reservados (<$author>, <$address> ...).
- **O texto excluído não desaparece de todos os lugares**: os instantâneos guardam versões anteriores de um documento (**Projeto > Excluir instantâneos antigos** os remove), e os backups guardam o projeto inteiro como era (seção 15). Os backups são arquivos ZIP comuns sem senha; uma pasta de backup dentro de uma pasta sincronizada é enviada pelo programa de sincronização.
- **Os seus arquivos não são criptografados pelo {{APP}}** (0.65): textos, notas, as configurações do projeto e os backups são arquivos comuns, legíveis por qualquer pessoa que possa ler a pasta em que estão. Para manter um manuscrito privado, mantenha o projeto e os seus backups em um disco criptografado (o instalador do Kubuntu oferece criptografar o disco inteiro; para um pen drive ou um disco externo, os Plasma Vaults do KDE ou uma unidade criptografada com LUKS), e mantenha a pasta de backup fora de pastas compartilhadas que você não controla.
- **Relatórios de falhas** ficam neste computador até que você copie ou envie um por conta própria (seção 20). Eles descrevem o estado do programa e nomeiam o projeto aberto e a sua pasta. No Windows, um despejo de memória é mantido ao lado de cada relatório; ele pode conter texto que você estava escrevendo, e o relatório avisa isso.

### Como adicionar o {{APP}} ao menu de aplicativos

Na primeira vez que você inicia o arquivo do programa ({{APP}}.AppImage), a página da primeira inicialização oferece adicionar o {{APP}} ao menu de aplicativos. Depois disso você pode iniciá-lo como qualquer outro programa, e um clique duplo em um projeto o abre. Nenhuma caixa confirma a entrada; uma falha é informada. A mesma página (e **Ajuda > Adicionar ao menu de aplicativos**) oferece **mover o AppImage para ~/Applications** (marcado no início quando o arquivo ainda não está lá), para que a entrada do menu o inicie a partir de um lugar fixo onde o arquivo permanece (a caixa de seleção diz isso com essas palavras, sem parênteses); se a mudança falhar (disco cheio, um arquivo com esse nome já existente), nada se perde: o arquivo permanece onde estava, a entrada aponta para lá, e uma mensagem diz o motivo. Se você mesmo mover o AppImage mais tarde, a entrada continua com o lugar antigo: use **Ajuda > Adicionar ao menu de aplicativos** de novo, que grava o lugar atual do arquivo (uma entrada mais antiga é substituída, não duplicada). Depois de uma mudança, as atualizações do programa, a cópia *.before-update* e **Ajuda > Solução de problemas e desenvolvimento > Informações do sistema** usam o novo lugar imediatamente.

O único comando do menu Ajuda, **Ajuda > Adicionar ao menu de aplicativos**, segue o estado da entrada: ele adiciona a entrada enquanto não há nenhuma e, quando já há uma, passa a se chamar *Remover do menu de aplicativos* e retira o que a adição fez (o arquivo da entrada, o ícone e os tipos de arquivo), deixando o AppImage onde está. Se você mover o arquivo do programa por conta própria, remova a entrada e adicione-a de novo.

### No Windows: tudo em uma pasta

A versão do {{APP}} para Windows (experimental, publicada desde 0.91.1) é uma pasta que você descompacta onde quiser, por exemplo *C:\Users\<your name>\Quillbinder*; ela não precisa de instalação nem de direitos de administrador. Ela guarda as suas configurações, complementos, atualizações do programa, fontes instaladas e relatórios de falhas na pasta *data* dentro dessa pasta, e nada nas pastas do próprio Windows nem no registro. A pasta inteira pode ser movida ou copiada (para um pen drive ou outro computador); excluí-la remove o programa. Uma pasta em que o programa não pode gravar (como *Program Files*) é recusada com uma mensagem que diz onde colocá-la em vez disso. As atualizações do programa são instaladas com **Ajuda > Complementos e atualizações > Instalar atualização ou complemento**; para passar a uma versão que você baixou de novo, descompacte-a e copie a pasta *data* para a nova pasta. Os seus projetos ficam onde você os guarda.

Há duas formas de obter essa pasta (0.90.4). O **programa de instalação** (*...-setup.exe*) pergunta por uma pasta e descompacta o {{APP}} nela; ele não precisa de direitos de administrador e não altera nada mais no computador: nenhuma entrada em Aplicativos, nada no registro, nenhuma entrada no menu Iniciar (o guia descrito abaixo oferece isso). Descompactar uma versão mais nova na mesma pasta mantém *data*, ou seja, as suas configurações e complementos. O **arquivo zip** contém a mesma pasta: descompacte-o por completo. O Windows pode avisar sobre qualquer um dos arquivos porque ele não é assinado (SmartScreen: *Mais informações*, depois *Executar assim mesmo*).

Duas coisas diferem do Linux por causa do próprio Windows (0.90.4). A tecla **AltGr** digita o seu caractere, como você espera: em um teclado alemão, AltGr+7, 8, 9, 0 e < dão { [ ] } e |, embora o {{APP}} tenha comandos em Ctrl+Alt com essas teclas; pressione as teclas Ctrl e Alt da esquerda para os comandos. E os arquivos necessários só por um instante (uma página da web salva descompactada para visualização, ou a sua cópia para o navegador) são mantidos na pasta temporária do Windows e removidos quando o {{APP}} é fechado. A Transcrição de fala usa o processador no Windows, não a placa de vídeo.

Na primeira inicialização, um guia diz onde o programa guarda os seus arquivos e avisa quando a pasta dele está dentro da pasta de um serviço de sincronização (OneDrive, Dropbox, Google Drive ...) ou tem um caminho longo. Ele também pede estas informações, que **Ajuda > Solução de problemas e desenvolvimento > Este computador** e **Arquivo > Opções > Geral > Este computador** mostram de novo:

- **Backups**: não há pasta de backup padrão. Até você escolher uma, nenhum backup automático é feito; a barra de rodapé do primeiro editor mostra **Sem backups** ao lado do texto de salvamento (clique nele para escolher a pasta), e o primeiro backup que seria executado pede uma pasta. O melhor é outra unidade.
- **Notas do Bloco de rascunho e transcrições**: em Documentos (o padrão), na pasta do programa ou em uma pasta sua. Uma mudança dá às duas ferramentas o novo lugar; o que elas gravaram antes permanece onde está.
- **Windows**: uma entrada no menu Iniciar, um atalho na área de trabalho e os tipos de arquivo de projeto (os arquivos *.qbx* abrem com o {{APP}}, e ele é oferecido em **Abrir com** para os arquivos *.scrivx* do Scrivener). Essas são as únicas coisas que o programa coloca fora da sua pasta, cada uma somente quando você a adiciona, e cada uma com **Remover**. Remova-as antes de excluir ou mover a pasta.

As fontes instaladas com **Formatar > Fonte > Instalar arquivos de fonte** ficam na pasta do programa e são só do {{APP}}; outros programas não as veem.

Os complementos com código de programa têm versões próprias para Windows (os nomes dos arquivos deles terminam em *-windows*): **Reprodução de mídia** reproduz o que o próprio Windows reproduz, **Formatos extras de vídeo e áudio** adiciona o FFmpeg, e **Transcrição de fala** escreve a fala no processador (as placas de vídeo ainda não são usadas no Windows). **Assistente de tradução** faz parte da versão completa também no Windows (0.92.0). Os complementos sem código de programa (temas, modelos, formatos de compilação) são os mesmos arquivos em todos os sistemas.

### Em tablets Android

A versão do {{APP}} para Android (experimental, publicada desde 0.91.3) é um aplicativo para tablets com Android 9 ou posterior (ARM de 64 bits). Ela tem as mesmas janelas que em um computador, feitas para os dedos: um dedo rola e toca, dois dedos dão zoom, um dedo que fica meio segundo parado sobre algo abre o seu menu de clique com o botão direito (em um texto, um toque no texto selecionado faz isso), e as barras de ferramentas se dobram em tantas linhas quantas a tela precisar. Baixe o arquivo APK no tablet e abra-o; o Android pergunta se o navegador ou o aplicativo Arquivos pode instalar aplicativos, o que você permite para esta instalação. Uma nova versão vem como um novo APK e é instalada por cima da antiga, mantendo as suas configurações e complementos; **Ajuda > Complementos e atualizações > Verificar atualizações agora** diz quando há uma e abre o download dela no navegador (complementos mais novos são instalados dentro do {{APP}}).

Com um teclado e um mouse externos (desde 0.92.0): Ctrl + roda do mouse dá zoom, e Ctrl + clique ou Shift + clique escolhe vários documentos, como em um computador. A linha entre dois painéis mostra uma alça; um mouse pressionado a alguns pixels ao lado dela a leva junto. O botão **Voltar** do Android fecha o menu, a dica ou a janela da frente; toda janela tem um botão que a fecha, e uma janela maior que a tela rola. Em um tablet, o alfinete do mapa mental fixado aparece na linha selecionada do Fichário (clique nele, ou use o menu de clique com o botão direito da linha: *Fixar o mapa mental deste item*, *Liberar o mapa mental fixado*), e um toque no cadeado de uma linha mostra quem está trabalhando nela. Um dedo na visão geral do mapa mental funciona como o mouse: um toque move a visão para lá, um dedo arrasta o retângulo da parte mostrada, e a alça no canto dela muda o seu tamanho. **Arquivo > Salvar como…** pergunta primeiro o formato em um tablet, depois abre a janela de arquivos.

Na primeira inicialização, o {{APP}} pede acesso aos arquivos do tablet. Com ele, os projetos vão para *Documents/Quillbinder*, os backups para *Documents/Quillbinder/Backups*, as exportações para *Documents/Quillbinder/Exports* e as notas do Bloco de rascunho para *Documents/Quillbinder/Scratch Pad*, onde outros aplicativos, um aplicativo de sincronização e um computador conectado por cabo podem alcançá-los; o botão abre a página do Android *Acesso a todos os arquivos*, na qual você o ativa para o {{APP}}. Sem ele, tudo fica na pasta do próprio aplicativo, que nenhum outro aplicativo vê e que o Android exclui junto com o aplicativo; *Arquivo > Arquivos neste tablet* pergunta de novo e diz onde estão as coisas. Quando o Android coloca o aplicativo de lado (o botão Início, outro aplicativo), o {{APP}} salva imediatamente.

Os dicionários de ortografia (inglês e alemão; os outros idiomas são complementos, veja a seção 4), as ferramentas de escrita, os importadores de páginas da web e de arquivos antigos do Word e (desde 0.92.0) o Assistente de tradução fazem parte do aplicativo; o Assistente de tradução é instalado na primeira inicialização de uma versão e só baixa um modelo de idioma quando você escolhe um. Os complementos para Android (os nomes dos arquivos deles terminam em *-android-arm64*) são instalados com **Ajuda > Complementos e atualizações > Instalar atualização ou complemento**; a janela de arquivos abre na pasta Downloads, e o {{APP}} inicia de novo quando o complemento precisa disso. Pacotes feitos para Linux ou Windows e atualizações do programa são recusados com o motivo: o próprio aplicativo só é atualizado por um novo APK. Ainda não há na versão para Android: Transcrição de fala, áudio e vídeo no editor e relatórios de falhas. O texto copiado dentro do {{APP}} mantém a sua formatação quando você o cola; a área de transferência do Android carrega apenas texto simples, de modo que o texto copiado para outros aplicativos chega sem formatação.

---

## 2. A janela principal

A janela tem três colunas:

| Área | O que ela faz |
|------|---------------|
| **Fichário** (esquerda) | O sumário de todo o seu projeto: cada documento e pasta. |
| **Editor** (meio) | Mostra o que você selecionou no Fichário: texto, um quadro de cortiça de fichas ou uma estrutura de tópicos. |
| **Inspetor** (direita) | Informações sobre o documento atual: sinopse, notas, favoritos, metadados, instantâneos e comentários. |

Na primeira vez que o {{APP}} inicia em uma tela mais estreita que 1600 px, a janela abre maximizada. O tamanho, o lugar e o estado da janela (maximizada ou não) são salvos um instante depois de cada mudança, de modo que uma falha não os perde. Oculte o Fichário com **Ctrl+Alt+B** e o inspetor com **Ctrl+Alt+I** para ganhar mais espaço. Isso vale para a janela mais larga que alta; uma janela mais alta que larga organiza as mesmas três áreas de outra forma (próxima seção). As cores e fontes das três áreas, e as do quadro de cortiça, da estrutura de tópicos e do modo de composição, vêm do tema (**Visualizar > Temas**) e podem ser alteradas área por área (seção 16), também com um clique com o botão direito na área (**Aparência**). Cada projeto mantém o seu próprio visual.

A **barra de ferramentas** no alto contém, da esquerda para a direita: o botão **Visualizar** (Fichário, coleções, inspetor, layouts), novo documento (com um menu para pastas, modelos e importação), nova pasta, lixeira, **desfazer** e **refazer**, o botão **buscar** (busca no projeto; a seta ao lado dele lista as outras ferramentas de busca: Buscar no documento, Substituir no projeto, Localizar por formatação), o campo de **busca rápida**, os três botões de visualização (texto, quadro de cortiça, estrutura de tópicos), favoritos do projeto, referência rápida, comentários e notas de rodapé, compilar, modo de composição, o **microfone** da Transcrição de fala (quando esse complemento está instalado) e o botão do inspetor. Você decide o que ela contém (veja "Barras de ferramentas próprias" abaixo).

A **barra de formatação** logo abaixo contém o estilo, a fonte, o tamanho, o espaçamento entre linhas, a formatação padrão do texto, a janela de fontes, negrito, itálico, sublinhado, riscado, realce, cor do texto, alinhamento, listas, o espaço antes e depois do parágrafo, o recuo da primeira linha e do parágrafo inteiro, comentários, notas de rodapé, anotações no texto e imagens. Mostre ou oculte as duas barras com **Visualizar > Barras de ferramentas**.

**Caixas de diálogo.** Uma janela que o programa abre (Opções, Compilar, Metas do projeto ...) nunca é maior que a tela sem os painéis da área de trabalho: em uma tela pequena de notebook, o conteúdo dela rola ou diminui, e os botões permanecem à vista. Quando ela fecha, o teclado volta para onde estava, geralmente o texto, de modo que você pode continuar digitando de imediato.

**Onde estou?** Dê um clique duplo no espaço ao lado do texto (o papel ao lado da coluna ou a área ao redor dela): o Fichário seleciona o documento ao lado do clique, abre as pastas acima dele e faz a sua linha piscar duas vezes em laranja. O editor mantém o que mostra. **Navegar > Mostrar no Fichário** (**Ctrl+Shift+R**) faz o mesmo para o documento no cursor.

O **rodapé** de cada editor mostra a contagem de palavras e de caracteres (selecione texto para contar apenas a seleção) e a meta do documento. Clique na contagem para ver detalhes. Quando o editor mostra uma pasta (ou um documento com subdocumentos), o rodapé também mostra o **total** da pasta: as palavras do texto dela e de tudo o que há dentro, por exemplo "Folder: 12,345 words". O quadro de cortiça e a estrutura de tópicos mostram o mesmo total no meio do seu rodapé; com vários itens mostrados, é o total de todos eles. Um clique em um total mostra os detalhes dele (palavras, caracteres, caracteres sem espaços, parágrafos e páginas), como a contagem de um texto. O inspetor o mostra em **Total** nos metadados de uma pasta, e parar o mouse sobre uma pasta no Fichário também o mostra (com as palavras da própria pasta quando ela tem texto próprio).

### Os menus

| Menu | O que ele contém |
|------|------------------|
| **Arquivo** | Projetos, salvamento, o formato do projeto ({{APP}} ou Scrivener), backups, importação e exportação, comparar e transferir com um arquivo do Word ou do LibreOffice, compilar, impressão, conversão de arquivos, opções |
| **Editar** | Desfazer, área de transferência, seleção, localizar e substituir (no texto, no projeto, por formatação), mover documentos no Fichário, ferramentas de escrita |
| **Inserir** | Imagens, tabelas, links para documentos e objetos, links da web, comentários, notas de rodapé, marcas de tempo de mídia, numeração automática, espaços reservados, caracteres especiais, data e hora |
| **Visualizar** | Os três modos de visualização, o editor dividido (**Dividir editor**), os lugares do Fichário, do editor e do inspetor e os layouts salvos (**Layout**), os temas de cores (**Temas**), Fichário, coleções e inspetor (e **Teclas de adição rápida no Fichário**), barras de ferramentas, opções de edição de texto (visualização de página, régua, rolagem de máquina de escrever), opções do quadro de cortiça e da estrutura de tópicos, cores de etiquetas, zoom, modo de composição, tela cheia |
| **Navegar** | Voltar e avançar, documento anterior e próximo, abrir documentos no outro editor, no suporte de cópia ou como referência rápida, a trava do editor, em qual editor o Fichário carrega, as abas do inspetor (todas elas, sob os nomes das suas dicas: **Sinopse e notas**, **Favoritos**, **Metadados**, **Instantâneos**, **Comentários e notas de rodapé**, o mapa mental, os objetos e as abas das ferramentas de escrita) |
| **Projeto** | Novos documentos, configurações do projeto (também salvas em um arquivo e carregadas de um arquivo), metas, palavras-chave, estatísticas, histórico de escrita, lixeira, exclusão de instantâneos antigos |
| **Documentos** | Instantâneos, renomear, duplicar, dividir, mesclar, converter, ícone, etiqueta, status, favoritos |
| **Formatar** | Fonte (com instalação de fontes), parágrafo, estilo, listas, realce, cópia de formatação, modo de revisão |
| **Janela** | Bloco de rascunho, janelas de referência rápida |
| **Ajuda** | **Localizar comando**, este manual, o Guia de início rápido, o projeto de boas-vindas, a lista de todos os espaços reservados, a lista de atalhos de teclado, a ajuda dos mapas mentais, o submenu **Complementos e atualizações** (Gerenciador de complementos, Instalar atualização ou complemento, a verificação de atualizações), adicionar o {{APP}} ao menu de aplicativos (e retirá-lo), o submenu **Solução de problemas e desenvolvimento** (informações do sistema, Este computador, relatórios de falhas, Abrir pasta de plugins, o guia do desenvolvedor), Relatar um problema, as licenças de terceiros, Sobre |

**Localizar comando** (**Ajuda > Localizar comando…**) abre uma janela com um campo de busca e a lista de todos os comandos dos menus, cada um com o seu menu e o seu atalho. Digite parte de um nome (várias palavras restringem a busca, em qualquer ordem); **Return** ou um clique duplo executa o comando escolhido. Um comando que está desativado no momento aparece em cinza e não é executado. Ele não tem atalho próprio.

Um submenu mostra nomes curtos: **Arquivo > Importar > Arquivos**, **Documentos > Dividir > na seleção**, **Navegar > Abrir > como referência rápida**. O atalho de um comando é mostrado ao lado dele e pode ser alterado (seção 19). Cada entrada de um menu tem uma letra própria (sublinhada quando você mantém **Alt** pressionada), de modo que **Alt** e as letras alcançam todos os comandos.

Sem um projeto aberto, os menus **Inserir**, **Navegar**, **Projeto**, **Documentos** e **Formatar** ficam em cinza, assim como os comandos dos outros menus que precisam de um texto ou de um Fichário (Localizar, Zoom, o layout do editor, Foco no Fichário ...). O que funciona sem um projeto: criar, abrir e importar projetos, converter arquivos, instalar atualizações e complementos, as opções, temas e layouts, as ferramentas de escrita que não precisam de projeto e a Ajuda.

### Os painéis conforme o formato da janela

A janela organiza o Fichário, o editor e o inspetor conforme o seu formato, e muda por conta própria quando o formato muda (você arrasta a janela para ficar mais alta que larga, ou gira um tablet). Uma janela quadrada conta como larga.

- **Janela larga** (mais larga que alta, ou quadrada): três colunas. Escolha a ordem delas em **Visualizar > Layout > Janela larga**: qualquer uma das seis ordens de Fichário, editor e inspetor. As larguras são definidas pelas linhas entre as colunas.
- **Janela vertical** (mais alta que larga): duas linhas abaixo das barras de ferramentas. Uma linha é o editor, na largura total da janela; a outra contém o Fichário e o inspetor lado a lado. Em **Visualizar > Layout > Janela vertical**, **Editor no meio** coloca a linha do editor acima da outra, **Editor embaixo** abaixo dela; **Fichário à esquerda** e **Fichário à direita** escolhem o lado do Fichário (o inspetor fica com o outro). As alturas das duas linhas e as larguras do Fichário e do inspetor são definidas pelas linhas entre eles. Com o teclado na tela visível (Android), a linha do Fichário e do inspetor mantém a sua altura; mova a linha se quiser outra.
- Um Fichário ou inspetor oculto deixa o seu espaço para o outro na linha vertical, em largura total; com os dois ocultos, o editor tem a janela inteira. Dois editores (**Visualizar > Dividir editor**) ficam dentro do lugar do editor.
- **Visualizar > Layout > Redefinir layout** devolve o formato que a janela tem agora aos seus valores iniciais: Fichário, editor, inspetor em uma janela larga; o editor no meio e o Fichário à esquerda em uma vertical.

Os dois grupos estão sempre no menu; cada um vale quando a janela tem aquele formato, e cada formato volta como você o deixou (lugares e tamanhos). As escolhas pertencem a este computador ou tablet e são mantidas nas configurações do programa, nunca em um projeto: um projeto que você compartilha entre computadores mantém a disposição de cada dispositivo, e nada sobre isso é gravado nos arquivos do projeto.

### Layouts

Um layout lembra como a janela está organizada: se o Fichário, as coleções, o inspetor, a barra de ferramentas e a barra de formatação são mostrados, a largura das colunas, como o editor está dividido, qual visualização cada editor usa e em qual editor o Fichário carrega. Ele não diz nada sobre documentos, de modo que cada layout funciona em todo projeto.

Escolha um layout em **Visualizar > Layout** ou no botão **Visualizar** da barra de ferramentas. Sete layouts vêm de fábrica:

| Layout | Disposição |
|--------|------------|
| **Padrão** | Fichário, um editor, inspetor |
| **Fichário e editor** | Fichário e texto, sem inspetor |
| **Somente o editor** | Apenas o texto com as duas barras |
| **Estrutura de tópicos e editor** | Estrutura de tópicos à esquerda, texto à direita. O Fichário carrega grupos na estrutura de tópicos; um clique em uma linha abre esse documento à direita |
| **Quadro de cortiça e editor** | Quadro de cortiça acima, texto abaixo, conectados da mesma forma |
| **Dois editores** | Dois textos lado a lado |
| **Planejamento** | Fichário, quadro de cortiça e o inspetor com sinopse e notas |

**Visualizar > Layout > Salvar layout atual** guarda a janela como está agora com um nome à sua escolha; marque "Lembrar também o tamanho da janela" se o layout deve redimensionar a janela. **Gerenciar layouts** mostra todos os layouts com uma pequena imagem e permite usar, atualizar (substituir pela janela atual), renomear, excluir, exportar e importar os seus próprios layouts. Os layouts de fábrica não podem ser alterados; salve uma janela alterada com um novo nome. Cada layout pode ter um atalho de teclado (seção 19, grupo "Layouts").

### As barras de ferramentas e os seus lugares

Desde 0.91, a janela tem oito barras de ferramentas em duas linhas (dez desde 0.95, com duas que começam desativadas). A primeira linha contém **Principal** (Visualizar, novo texto, nova pasta, lixeira, Desfazer, Refazer), **Temas** (o botão de tema, desativado no início), **Perfis** (o perfil em uso e um menu para trocá-lo, desativado no início; desde 0.95), **Buscar** (o botão de busca, o campo de busca rápida, os modos de visualização) e **Ferramentas** (favoritos, Referência rápida, comentários, Compilar, Mostrar invisíveis, Modo de composição, os botões das ferramentas de escrita, o inspetor). A segunda linha contém **Estilo e fonte**, **Caractere e alinhamento**, **Parágrafo e inserção**, a **barra de ícones** e **Etiquetas** (um botão para cada cor de etiqueta do projeto aberto e **Sem etiqueta**; desativada no início; desde 0.95).

- **Mover uma barra de ferramentas.** Cada barra de ferramentas tem uma alça na frente (duas colunas de pontos; o ponteiro vira uma cruz com setas sobre ela). Arraste a alça, e a barra de ferramentas vai para onde você a levar:
  - **Ao longo da sua linha.** Enquanto o ponteiro fica entre os meios das duas barras vizinhas, a barra o segue de imediato. O espaço livre antes de uma barra tem a largura que você der. Movendo para a direita, a barra primeiro toma o espaço livre depois dela; quando chega à próxima barra, empurra-a junto, até onde as barras depois dela cabem juntas no fim da linha. Movendo para a esquerda, ela para onde a barra anterior termina, e as barras depois dela ficam onde estão (arraste-as até ela se quiser que fiquem juntas de novo). A primeira barra de uma linha fica na borda da janela.
  - **Para outro lugar na ordem.** Leve o ponteiro além do meio de uma barra vizinha, ou sobre qualquer outra barra da linha. A barra fica onde está por enquanto, e uma linha na cor de destaque mostra onde ela ficará; solte o botão do mouse para colocá-la ali. Assim qualquer ordem é possível, também para a primeira e a última barra de uma linha.
  - **Para outra linha.** Leve o ponteiro para a outra linha: a linha mostra o lugar ali. Qualquer barra pode ficar em qualquer linha, uma barra de formatação ao lado da barra principal ou a barra de busca entre as barras de formatação.
  - **Para uma linha própria, ou para a borda de baixo.** Leve o ponteiro logo abaixo das linhas de barras (aparece uma linha atravessando a janela) para abrir uma nova linha, ou para a borda de baixo da janela para colocar a barra ali. A borda de baixo aceita barras lado a lado e em linhas, como o alto.
  - **Esc**, ou soltar onde nenhuma linha aparece (no meio da janela, por exemplo), deixa tudo como estava.

  As barras depois do lugar antigo se aproximam, e o espaço livre que havia diante da barra permanece na sua linha; no novo lugar, a barra fica logo depois da barra anterior, e você pode puxá-la mais para a direita a partir dali. Em uma janela estreita demais para o espaço livre que você deixou, o espaço encolhe antes de uma barra perder botões; ele volta quando a janela fica larga de novo. Uma barra desativada mantém o seu lugar. Uma barra de ferramentas não pode ficar nas laterais nem flutuar como janela própria. O {{APP}} lembra as linhas, a ordem e os espaços.
- **Desativar uma barra de ferramentas.** **Visualizar > Barras de ferramentas** tem uma chave para cada barra; a mesma lista aparece com um clique com o botão direito em uma barra ou no espaço livre ao lado das barras. **Mostrar barra de ferramentas** e **Mostrar barra de formatação** ligam ou desligam uma linha inteira como antes (as quatro barras da primeira linha, as três barras de formatação), e os layouts usam essas duas; a chave de uma barra isolada fica indisponível enquanto a sua linha está desligada.
- **Visualizar > Barras de ferramentas > Redefinir posições das barras de ferramentas** devolve todas as barras aos seus lugares padrão (as duas linhas na ordem padrão, nada na borda de baixo) e fecha os espaços entre elas.

### Barras de ferramentas próprias

**Visualizar > Barras de ferramentas > Personalizar barras de ferramentas** (ou um clique com o botão direito em uma barra) abre uma janela com duas listas: os comandos disponíveis à esquerda, o conteúdo da barra à direita. Escolha a barra de ferramentas no alto.

- Adicione um comando com o botão de seta ou um clique duplo. Todo comando dos menus está disponível, também os das ferramentas de escrita (Editar > Ferramentas de escrita); o campo de busca acima da lista o encontra.
- Remova um botão com a outra seta ou um clique duplo, e mova-o com os botões para cima e para baixo ou arrastando.
- **Separador** e **Espaço flexível** podem ser usados quantas vezes você quiser. Um espaço flexível empurra o que vem depois para a direita; dois deles centralizam o que está entre eles.
- Algumas entradas são mais de um comando: **Visualizar**, **Adicionar**, **Buscar**, o **campo de busca rápida**, **Modo de visualização**, **Favoritos do projeto**, **Comentários e notas de rodapé**, **Layouts** e **Temas** (o botão de tema com o seu menu) para as quatro barras da primeira linha; as listas de estilo, fonte, tamanho e espaçamento entre linhas e os quatro botões de parágrafo para as três barras de formatação. Um controle assim existe uma vez: adicionado a uma barra, ele sai da barra que o tinha e vai para lá. Um controle da primeira linha não pode ir para uma barra de formatação, nem o contrário.
- Abaixo das listas você escolhe ícones pequenos, médios ou grandes e se as barras da primeira linha mostram texto ao lado ou abaixo dos ícones.
- **Conjunto padrão** devolve à barra os botões com que o programa veio.

Se uma barra contém mais do que cabe na janela, uma seta no fim dela abre o resto. Uma barra de ferramentas que fica sem nenhum botão é desativada; ativada de novo em **Visualizar > Barras de ferramentas**, ela tem o seu conjunto padrão. Uma barra que você organizou em uma versão anterior a 0.91 foi dividida entre as novas barras: cada botão foi para a barra que o contém nos conjuntos padrão, e o que você havia tirado continua fora.

### A barra de ícones

A **barra de ícones** contém até dez ícones de documento que você usa com frequência. Um clique em um deles dá esse ícone aos documentos e pastas selecionados (no Fichário, ou no quadro de cortiça ou na estrutura de tópicos em que você está trabalhando); o botão **↺** depois deles devolve o ícone padrão, e o último botão escolhe os ícones da barra. Mostre ou oculte a barra com **Visualizar > Barras de ferramentas > Barra de ícones**.

**Visualizar > Barras de ferramentas > Escolher ícones para a barra de ícones…** (também no menu de clique com o botão direito da barra) abre uma janela com todos os ícones à esquerda, inclusive os ícones de projeto que são seus, e os ícones da barra à direita. Adicione com **Adicionar →** ou um clique duplo, remova com **← Remover** ou um clique duplo, e mude a ordem com **Para cima** e **Para baixo**. **Conjunto padrão** devolve os dez ícones com que o programa veio (Livro verde, Revisado, Marcado, Desmarcado, Ponto de interrogação, Bandeira vermelha, Bloco de notas amarelo, Ficha de personagem, Ficha de local, Aviso).

---

## 3. O Fichário

Todo projeto tem três pastas especiais:

- **Rascunho** (em alguns modelos *Manuscrito*): tudo o que vai se tornar o seu livro. Só o que está dentro do Rascunho é compilado.
- **Pesquisa**: notas, fichas de personagens, imagens, PDFs e qualquer outra coisa que você reúna.
- **Lixeira**: os itens excluídos esperam aqui até você escolher **Projeto > Esvaziar a Lixeira**. A pergunta dele, como toda pergunta antes de algo que não pode ser desfeito, nomeia o passo no seu botão e tem **Cancelar** como padrão, de modo que a tecla Enter nunca exclui nada (0.70).

### Como trabalhar com itens

- **Novo documento**: **Ctrl+N** (ou o botão ➕). Ele é adicionado abaixo do item selecionado. Um documento chamado "Untitled" (ou sem título) assume a primeira linha do seu texto como título assim que você escreve: no máximo 60 caracteres, cortado em uma palavra, sem anotações no texto, notas de rodapé no texto e imagens. O título acompanha a primeira linha até você mesmo renomear o documento; um documento cujo texto você exclui volta a ser "Untitled". Os documentos "Untitled" que já têm texto recebem o nome quando o projeto abre. O título é um título comum: é salvo, compilado (quando os títulos são compilados, uma primeira linha que é um título aparece duas vezes) e gravado nos projetos do Scrivener.
- **Nova pasta**: **Alt+Shift+N**.
- **Renomear**: **F2**, ou dê um clique duplo lento no título.
- **Mover**: arraste os itens com o mouse, ou use **Ctrl+Alt+setas**: para cima e para baixo movem dentro da pasta; para a esquerda move para fora da pasta; para a direita move para dentro do item acima. Solto *entre* dois itens, um documento vai para ali; solto *sobre* um item, ele se torna subdocumento dele. Se você aninhar documentos por acidente, desative o soltar sobre documentos de texto em **Arquivo > Opções > Comportamentos** (as pastas sempre aceitam o que é solto nelas).
- **Abrir ou fechar um ramo inteiro**: mantenha **Alt** pressionada e clique na seta de uma pasta ou grupo.
- **Excluir**: pressione **Delete**. O item vai para a Lixeira; um quadro de cortiça ou uma estrutura de tópicos que o mostrava como um dos vários documentos escolhidos o deixa sair de imediato (0.94.0).
- **Duplicar**: **Ctrl+D**.
- **Dividir e mesclar**: **Documentos > Dividir > na seleção** (**Ctrl+K**) corta o documento no cursor em dois; não funciona dentro de uma tabela nem da legenda de uma imagem (ponha o cursor antes ou depois dela). **Documentos > Mesclar** (**Ctrl+Shift+M**) junta os documentos selecionados no primeiro, com as suas sinopses, palavras-chave e notas; os outros vão para a Lixeira.
- **Alterar o ícone**: clique com o botão direito > **Alterar ícone** (ou **Documentos > Alterar ícone**), ou a barra de ícones (seção 2). Veja "Ícones" abaixo.

**Clique com o botão direito.** Em um documento, o menu de clique com o botão direito tem os comandos para os documentos selecionados (renomear, duplicar, mover para a Lixeira, instantâneos, etiqueta, status e mais). No espaço vazio abaixo dos documentos, ele oferece apenas o que cria algo novo: **Novo texto**, **Nova pasta**, **Novo a partir de modelo** e a **Aparência** do Fichário.

Um documento pode ter subdocumentos, exatamente como uma pasta. Pastas e documentos se comportam da mesma forma; as pastas apenas têm outra aparência e abrem como um quadro de cortiça por padrão.

**As teclas do Fichário (0.94.0).** Com o teclado no Fichário, **Return** abre o documento selecionado no editor e leva o teclado para lá (como faz um gerenciador de arquivos), e as setas percorrem a lista. **Espaço** faz parte da digitação de um nome: digite as primeiras letras de um título, espaços também ("Capítulo t"), e a seleção salta para o primeiro título que começa assim. A entrada **Visualizar > Teclas de adição rápida no Fichário** está desativada no início (também depois de uma atualização). Ativada, o Fichário tem as teclas de adição rápida: **Return** cria um novo documento vazio abaixo do selecionado e **Espaço** abre os documentos selecionados como painéis de Referência rápida (e na lista de uma coleção ou de uma busca).

Um clique no Fichário carrega o documento no editor. Depois de **Voltar**, de um link de documento ou com um editor travado, o editor pode mostrar outro documento que não o selecionado no Fichário: uma **barra cinza** marca então no Fichário o documento do editor. **Navegar > Mostrar no Fichário** (**Ctrl+Shift+R**) o seleciona.

**Fichário estreito, pastas profundas.** Cada nível de pastas é recuado em 12 px (metade do que era; altere em **Arquivo > Opções > Aparência > Fichário > Recuo por nível**). Quando os títulos são mais longos que a largura do Fichário, aparece uma barra de rolagem na parte de baixo do Fichário, de modo que você pode ler o fim de títulos longos sem alargar o Fichário.

### Ícones

**Documentos > Alterar ícone** (também no menu de clique com o botão direito do Fichário, do quadro de cortiça e da estrutura de tópicos) é construído como o menu de ícones do Scrivener 3 para Windows:

- **Redefinir ícone para o padrão**: o ícone padrão de um documento, pasta ou arquivo volta.
- **Ícone a partir de texto…**: cria um ícone de até três caracteres (por exemplo *PR*, *Cap1* ou *A*). Escolha a cor da caixa, das letras e da borda; a pequena imagem mostra o resultado. O ícone é mantido no projeto e dado de imediato aos documentos selecionados.
- **Gerenciar ícones…**: os ícones deste projeto. **Adicionar imagem…** transforma imagens (PNG, SVG, JPEG e outras) em ícones, **A partir de texto…** é o mesmo que acima, **Renomear…** e **Excluir** alteram a lista (documentos com um ícone excluído recebem de volta o ícone padrão), **Usar nos documentos selecionados** dá o ícone escolhido aos documentos selecionados. Os ícones são guardados no projeto (pasta *icons*), de modo que viajam com ele.
- **Recentes**: os últimos cinco ícones que você escolheu.
- **Livro**, **Personagens**, **Bandeira**, **Locais**, **Notas**, **A fazer**, **TV**, **Ponto de parada**: submenus com os ícones de cada grupo em várias cores (por exemplo Livro azul, verde, vermelho e amarelo; sete bandeiras; cinco blocos de notas; Marcado e Desmarcado, Aprovado, Reprovado e Incerto em A fazer).
- Depois os ícones isolados: Fim do livro, Quadro-negro, Calendário, Claquete, Relógio, Engrenagem, Olho, Figuras e gráficos, Arquivo, Início do livro, Informação, Púlpito, Lâmpada, Lupa, Música, Notas musicais, Estrutura de tópicos, Lápis, Revisado, Ponto de interrogação, Comentários do revisor, Tesoura, Balão de fala, Aspas, Estatísticas, Pasta sincronizada, Tabelas, Tubo de ensaio, Balão de pensamento e Aviso.
- **Ícones do projeto**: os seus próprios ícones (quando o projeto tem alguns), e **Mais ícones (área de trabalho)**: os ícones do tema da área de trabalho que as versões anteriores ofereciam.

**Cor da etiqueta nos ícones.** As partes brancas de muitos ícones (as folhas de um bloco de notas, o pano de uma bandeira branca, o papel de uma ficha, a caixa de um ícone feito de texto) assumem a cor da etiqueta do documento. Ative ou desative isso com **Visualizar > Usar cor da etiqueta em > Ícones** (ou **Arquivo > Opções > Aparência > Fichário**).

**Ícones e Scrivener.** Os ícones do {{APP}} são desenhados novamente, mas cada um deles leva o nome do ícone do Scrivener que representa. Um projeto no formato do Scrivener (seção 14) mantém esses nomes, de modo que o Scrivener mostra o seu próprio ícone para eles; os ícones feitos no Scrivener voltam aqui como o ícone correspondente. Os seus próprios ícones vão para a pasta *Icons* do *.scriv*. Um ícone para o qual este programa não tem imagem mantém o seu nome e é mostrado aqui como o ícone mais próximo.

### Etiquetas, status e "Incluir na compilação"

Clique com o botão direito em um item para definir a sua **etiqueta** (uma cor, por exemplo para o personagem do ponto de vista) e o seu **status** (por exemplo *Primeiro rascunho* ou *Concluído*). Altere as listas em **Projeto > Configurações do projeto**.

Um documento ou pasta com uma etiqueta mostra a cor dela também no inspetor: o cartão da sinopse e as notas do documento recebem uma leve tonalidade da cor da etiqueta (15 % da sua saturação), de modo que você vê de relance qual etiqueta o item tem. Itens sem etiqueta, ou com uma cinza, mantêm a cor normal do cartão.

A barra de ferramentas **Etiquetas** (**Visualizar > Barras de ferramentas > Etiquetas**, desativada no início) mostra lado a lado todas as cores de etiqueta do projeto aberto, com um botão para nenhuma etiqueta. Um clique define na hora a etiqueta dos documentos selecionados, como **Documentos > Etiqueta**; a dica de cada cor nomeia a etiqueta. As etiquetas com que um novo projeto começa são definidas em **Arquivo > Opções > Aparência > Cores padrão**.

Itens com **Incluir na compilação** desativado ficam no Rascunho, mas são deixados de fora quando você compila.

### Coleções

As coleções são listas extras de documentos, por exemplo "Precisa de pesquisa" ou "Cenas com a Ana". Mostre-as com o botão **Coleções**. Use o botão ➕ acima do Fichário para criar uma coleção a partir dos documentos selecionados, ou clique com o botão direito em documentos e escolha **Adicionar à coleção**. Remover um documento de uma coleção não o exclui. As buscas salvas (seção 9) também são coleções; elas se atualizam sozinhas.

- **Adicione documentos** arrastando-os do Fichário (ou do ícone do cabeçalho de um editor, de um quadro de cortiça, de uma estrutura de tópicos) para a aba da coleção.
- **Dentro da lista** de uma coleção, arraste documentos para a ordem que você quiser (a ordem do Fichário não muda), solte documentos em um lugar e pressione **Delete** para tirar os documentos selecionados da coleção. As coleções de busca não podem ser alteradas assim.
- **Arraste as abas** para colocar as coleções em outra ordem.
- Os documentos na lista podem ser arrastados como os do Fichário, e **Espaço** abre os selecionados como painéis de Referência rápida.

---

## 4. Como escrever

### Formatação de texto

| Ação | Atalho |
|------|--------|
| Negrito / itálico / sublinhado | Ctrl+B / Ctrl+I / Ctrl+U |
| Realce | Ctrl+Shift+H |
| Sobrescrito / subscrito | Ctrl+Shift++ / Ctrl+= (layout alemão: Ctrl+Shift+D para subscrito, veja "Atalhos e o layout do teclado" abaixo) |
| Alinhar à esquerda / centralizar / alinhar à direita | Ctrl+{ / Ctrl+\| / Ctrl+} (layout alemão: Alt+Shift+L / Alt+Shift+E / Alt+Shift+R) |
| Janela de fontes | Ctrl+T |
| Tamanho da fonte maior / menor | Ctrl+> / Ctrl+< |
| Janela de parágrafo | Ctrl+Alt+P |
| Copiar formatação / colar formatação | Ctrl+Alt+C / Ctrl+Alt+V |
| Aumentar zoom / reduzir zoom / redefinir zoom | Ctrl++ / Ctrl+- / Ctrl+0 |
| Zoom na área sob o mouse | Ctrl+roda do mouse |
| Mostrar ou ocultar a régua | Ctrl+Alt+R |
| Colar sem formatação | Ctrl+Shift+V |
| Colar com formatação (de outros programas) | Ctrl+Alt+Shift+V |
| Desfazer / refazer | Ctrl+Z / Ctrl+Shift+Z (veja "Desfazer e refazer" abaixo) |

Negrito, itálico, sobrescrito e semelhantes funcionam na seleção. Sem uma seleção, dentro de uma palavra eles alteram a palavra inteira; no fim de uma palavra (ou sem nada ao redor do cursor) eles alteram o que você digita em seguida: digite "H", pressione Ctrl+=, digite "2", pressione Ctrl+= de novo e digite "O" para obter H₂O.

Os **estilos** (Título 1–6, Título, Subtítulo, Citação em bloco e os seus próprios) são escolhidos na lista à esquerda da barra de formatação. Os estilos dizem à Compilação o que um parágrafo é, portanto use-os em vez de deixar o texto grande e em negrito manualmente. Veja "Estilos" abaixo.

**O que a barra de formatação mostra.** A barra mostra o texto como você o vê, para toda a seleção. **Negrito**, **Itálico**, **Sublinhado**, **Riscado**, sobrescrito e subscrito ficam ativados quando todo o texto selecionado tem essa aparência, também quando o estilo dele a produz (o negrito de um título). Um clique alterna o que você vê: **Negrito** em um título em negrito o deixa sem negrito, e ele continua sem negrito quando o projeto é salvo e aberto de novo. A fonte, o tamanho, o espaçamento entre linhas, os botões de alinhamento e a lista de estilos também mostram a seleção; quando a seleção contém duas fontes, dois tamanhos, dois alinhamentos ou parágrafos de dois estilos, esse campo fica vazio (nenhum botão de alinhamento fica ativado). Um tamanho digitado no campo vazio dá esse tamanho a toda a seleção.

**Listas**: use os botões de lista, ou digite "- " ou "1. " no início de um parágrafo.

**Tipografia inteligente**: aspas retas viram aspas curvas, e dois hifens viram um travessão enquanto você digita. Desative isso em **Arquivo > Opções > Edição**.

**Modo de revisão** (**Formatar > Modo de revisão**): tudo o que você digita é colorido com uma das cinco cores de revisão, para que você veja o que mudou em uma rodada de edição. Escolha **Desativado** para parar.

**Espaçamento e recuos na barra de formatação.** Quatro botões depois dos botões de lista definem os parágrafos que contêm o cursor ou a seleção: o **espaço antes** e o **espaço depois** do parágrafo (0, 3, 6, 9, 12 ou 18 pt), o **recuo da primeira linha** e o **recuo do parágrafo inteiro** (em centímetros ou polegadas, como a régua os mostra). O valor do parágrafo no cursor aparece marcado; **Outro...** pede qualquer valor. **Editar > Desfazer** desfaz uma alteração. Uma barra de formatação que você organizou por conta própria recebe os quatro botões uma vez, depois dos botões de lista; **Visualizar > Barras de ferramentas > Personalizar barras de ferramentas** os move ou remove.

**Desfazer e refazer.** **Editar > Desfazer** e **Editar > Refazer** funcionam no campo de texto que tem o teclado (o editor, a sinopse, as notas, um título) e dizem o que vão desfazer: "Desfazer digitação", "Desfazer colagem", "Desfazer negrito", "Refazer exclusão". Com o teclado em outro lugar, por exemplo no Fichário, eles ficam em cinza: mover documentos para a Lixeira, esvaziar a Lixeira e excluir instantâneos não podem ser desfeitos assim (um documento na Lixeira volta ao ser arrastado para fora; veja a seção 11 para os instantâneos). A seta ao lado dos botões **Desfazer** e **Refazer** da barra de ferramentas lista os passos, o próximo primeiro; escolher um desfaz (ou refaz) esse passo e todos os passos anteriores a ele na lista. Cada comando do menu Formatar é um passo.

**Cor do texto e realce na barra de formatação.** Os dois botões com uma barra colorida (uma caneta para o realce, uma letra *T* para a cor do texto) funcionam como os do Scrivener e do Word: um clique aplica a cor mostrada na barra ao texto selecionado ou, sem seleção, ao que você digita em seguida. Pare o mouse sobre um botão por um instante e uma pequena janela com **vermelho, azul, amarelo, verde, branco e preto** abre abaixo dele, além de **Sem realce** (ou **Automática** para a cor do texto) e **…** para qualquer outra cor. Um clique em uma cor a aplica e a torna a cor do botão, que a guarda para o próximo clique (também depois de reiniciar). Um realce é desenhado exatamente na cor que você escolheu, tanto em uma página clara quanto em uma escura. Onde o texto sobre ele seria difícil de ler (abaixo de um contraste de 4,5 : 1), o texto é mostrado escuro ou claro só na tela; a cor guardada dele, e o realce em arquivos salvos e compilados, permanecem como escolhidos.

### Fontes

Todo o texto de um projeto tem uma aparência: a **formatação padrão** (veja abaixo). Sobre ela, você pode dar a qualquer trecho de texto uma fonte própria.

- **Barra de formatação**: a lista de fontes e o campo de tamanho alteram o texto selecionado de imediato. Sem uma seleção, eles alteram o **parágrafo inteiro** que contém o cursor (e o que você digita em seguida ali). As setas do campo de tamanho alteram o texto a cada clique; um tamanho que você digita passa a valer com **Enter**. A barra de formatação, o menu Formatar e os seus atalhos agem sobre o texto em que o cursor esteve por último: um editor, as notas no inspetor, o texto dos favoritos no inspetor ou um campo de texto longo de um objeto (ali: negrito, itálico, sublinhado, riscado, cor do texto e realce). Um clique no campo de tamanho não altera isso.
- Depois de uma alteração na barra de formatação, uma barra acima do editor oferece o mesmo para o projeto inteiro: em um parágrafo com estilo (um título, por exemplo), **Alterar o estilo “Título 1”** dá ao estilo a nova fonte ou o novo tamanho, e todo o texto desse estilo em todos os documentos o acompanha; em um parágrafo sem estilo, **Usar em todo o texto** a torna a fonte ou o tamanho do projeto (veja "Formatação padrão" abaixo). Clique em **×**, ou simplesmente continue escrevendo em outro lugar, para manter a alteração onde está. Todo documento que muda recebe antes um instantâneo.
- **Formatar > Fonte > Mostrar fontes…** (**Ctrl+T**) abre a janela de fontes: fonte, estilo tipográfico (leve, regular, negrito, itálico e o que mais a fonte oferecer), tamanho, sublinhado, riscado, maiúsculas (versaletes ou todas em maiúsculas), espaçamento entre letras em porcentagem e cor do texto. Só o que você altera na janela é aplicado, de modo que uma seleção com palavras em negrito e comuns as mantém. **Fonte do projeto** devolve o texto à fonte, ao tamanho e à cor do projeto.
- **Negrito, Itálico, Sublinhado, Riscado**: um botão fica ativado quando *todo* o texto selecionado tem esse formato, não importa como você o selecionou (também da direita para a esquerda). Uma seleção que está em negrito só em parte mostra Negrito desativado, e **Negrito** então deixa tudo em negrito; um segundo clique deixa tudo comum. Na janela de fontes, um estilo tipográfico que você escolhe também é aplicado a toda a seleção.
- **Formatar > Fonte > Maior / Menor** (**Ctrl+>** / **Ctrl+<**) alteram o tamanho em um ponto; cada parte da seleção mantém o seu próprio tamanho e cresce ou encolhe.
- **Formatar > Fonte > Cor do texto…** e **Cor de texto automática**.

Escolher a fonte ou o tamanho do próprio projeto é guardado como "nenhuma fonte própria": esse texto acompanha o projeto quando você altera a formatação padrão depois.

**Fontes com vários designs.** Algumas fontes guardam vários designs de um mesmo tipo sob um só nome: a Sitka do Windows 11, por exemplo, contém os tamanhos ópticos Small, Text, Subheading, Heading, Display e Banner (cada um desenhado para a sua própria faixa de tamanhos, os maiores mais estreitos e com letras minúsculas menores), e algumas fontes contêm versões condensadas ou largas da mesma maneira. O {{APP}} lista cada design como uma fonte própria (por exemplo **Sitka Small**, **Sitka Text**, **Sitka Banner**), apenas com os estilos tipográficos desse design, de modo que negrito e itálico permanecem dentro do design que você escolheu; antes de 0.40.1, o texto em negrito podia sair em outro design e parecer menor e mais estreito. Os arquivos de fonte não são alterados. A fonte com o nome original mantém o seu próprio design, de modo que o texto composto nela antes mantém a sua aparência.

**Como instalar fontes.** **Formatar > Fonte** tem os comandos para fontes que ainda não estão no seu computador:

- **Instalar arquivos de fonte…**: escolha arquivos de fonte (TTF, OTF, TTC, PFB, WOFF ou um arquivo ZIP que os contenha). Eles são copiados para a sua própria pasta de fontes (*~/.local/share/fonts*), e as listas de fontes do {{APP}} os conhecem de imediato; outros programas os veem depois da próxima inicialização. Nenhuma senha é necessária, e as fontes são só do seu usuário.
- **Abrir pasta de fontes**: abre essa pasta no gerenciador de arquivos; as fontes que você mesmo colocar ali são encontradas depois de **Recarregar lista de fontes**.
- **Abrir gerenciador de fontes da área de trabalho**: abre o gerenciador de fontes do KDE (Configurações do sistema > Texto e fontes > Gerenciamento de fontes) ou o programa de fontes de outra área de trabalho, onde você também pode instalar fontes para todos os usuários e remover fontes.
- **Recarregar lista de fontes**: procura fontes novas e as nomeia.

### Formatação de parágrafo

**Formatar > Parágrafo > Parágrafo…** (**Ctrl+Alt+P**) define, para os parágrafos que contêm o cursor ou a seleção: alinhamento, recuo à esquerda e à direita, recuo da primeira linha (um valor negativo dá um recuo deslocado), espaço antes e depois, espaçamento entre linhas (um múltiplo da altura da linha, uma altura exata ou uma altura mínima) e paradas de tabulação (esquerda, centralizada, direita, ponto decimal). Só o que você altera é aplicado. **Redefinir para o padrão** remove a formatação que você definiu manualmente; o parágrafo volta a seguir o seu estilo e o projeto.

Formas rápidas: os botões de alinhamento, a lista de espaçamento entre linhas na barra de formatação, **Formatar > Parágrafo > Espaçamento entre linhas** e **Aumentar recuo / Diminuir recuo**.

Aplicar um estilo de parágrafo substitui a formatação definida manualmente nos parágrafos que ele alcança: recuos, espaçamento, alinhamento e paradas de tabulação, e as fontes, os tamanhos, as cores, o espaçamento entre letras e as maiúsculas do texto neles. Negrito, itálico, sublinhado, riscado, realces e estilos de caractere permanecem. Selecione texto em vários parágrafos para dar o estilo a todos eles, a cada parágrafo inteiro; uma seleção que termina exatamente no início de um parágrafo (por exemplo depois de **Shift+Down**) deixa esse parágrafo em paz.

**Formatar > Copiar formatação** (**Ctrl+Alt+C**) memoriza a formatação de fonte e de parágrafo no cursor; **Colar formatação** (**Ctrl+Alt+V**) a aplica à seleção, ou ao parágrafo inteiro quando nada está selecionado.

### Caracteres invisíveis

**Visualizar > Edição de texto > Mostrar invisíveis** (também o botão ¶ da barra de ferramentas) mostra os caracteres que você não vê: um ponto para cada espaço, uma seta para uma tabulação, um pequeno círculo para um espaço não separável, uma seta dobrada para uma quebra de linha (**Shift+Enter**) e ¶ no fim de cada parágrafo. Eles são desenhados somente na tela, no texto principal de cada editor; nunca fazem parte do texto, de uma impressão nem da compilação. A escolha é mantida. A cor deles é a do texto, esmaecida em direção ao papel; uma cor própria é definida em **Arquivo > Opções > Aparência > Marcas textuais** (ou clique com o botão direito no texto > **Aparência**), e ela pertence à aparência do projeto como o papel e o texto (o modo de composição mantém a cor esmaecida do texto).

### A régua

**Visualizar > Edição de texto > Mostrar régua** (**Ctrl+Alt+R**) mostra uma régua acima do texto. Ela pertence ao parágrafo que contém o cursor.

- O marcador no alto é o **recuo da primeira linha**, os marcadores embaixo são o **recuo à esquerda** e o **recuo à direita**. Arraste-os. O recuo à esquerda leva a primeira linha junto.
- **Clique** na metade de baixo da régua para adicionar uma parada de tabulação. Arraste uma parada de tabulação para movê-la; arraste-a para cima ou para baixo, para fora da régua, para removê-la. **Clique com o botão direito** em uma parada de tabulação para torná-la esquerda, centralizada, direita ou decimal.
- Os marcadores se encaixam em quartos de centímetro (dezesseis avos de polegada); mantenha **Shift** pressionada para posicioná-los livremente.
- **Clique duplo** na régua abre a janela de parágrafo. O menu de clique com o botão direito alterna entre centímetros e polegadas (também em **Arquivo > Opções > Edição**).

As paradas de tabulação são medidas a partir do início da primeira linha do parágrafo. Sem paradas de tabulação próprias, um parágrafo tem uma posição de tabulação a cada 2,1 cm.

A régua mostra comprimentos reais. O zoom aumenta as letras na tela, mas não os recuos e as paradas de tabulação, portanto compare distâncias a 100 %.

### Formatação padrão

- **Este projeto**: **Projeto > Configurações do projeto > Formatação** (também o botão de controles deslizantes na barra de formatação) define fonte, tamanho, espaçamento entre linhas, recuo da primeira linha, espaço depois dos parágrafos e o alinhamento do texto do projeto inteiro (quatro botões: esquerda, centralizado, direita, justificado). No Painel de estilos, **Sem estilo** > **Redefinir a partir da seleção** toma esses valores do parágrafo no cursor (veja "Estilos" abaixo). **Aplicar** altera todos os documentos de uma vez e deixa a janela aberta, para que você veja o resultado atrás dela; **OK** faz o mesmo e fecha a janela. **Usar atual** toma os valores do parágrafo que contém o cursor.
  - Com **Alterar também o texto que tem fonte, tamanho ou espaçamento próprio** (ativado por padrão), o texto que foi formatado manualmente ou veio com fonte própria de uma importação também acompanha, desde que tenha a fonte, o tamanho e o espaçamento principais do seu documento. O que difere do texto principal permanece como está: uma linha de título maior, uma passagem em outra fonte, um parágrafo sem recuo. Desative a caixa para deixar em paz todo o texto formatado manualmente.
  - Todo documento que muda recebe antes um instantâneo (inspetor, aba de instantâneos, "Before the project formatting changed"), para que você possa voltar. Os documentos de roteiro mantêm o seu próprio formato.
- **Novos projetos**: **Arquivo > Opções > Edição > Formatação de novos projetos**, com os mesmos quatro botões de alinhamento e **Usar formatação do editor atual**. Os projetos existentes mantêm a sua formatação. O padrão é 11,0 pt, espaçamento entre linhas 1,09, sem recuo da primeira linha e 6 pt depois dos parágrafos. Os projetos importados do Scrivener também começam com esses valores (com a fonte mais usada no projeto do Scrivener).
- **De volta ao padrão**: selecione documentos no Fichário e escolha **Documentos > Converter > Texto para a formatação padrão…**. Você decide se fontes e cores, recuos e espaçamento, alinhamento e paradas de tabulação são removidos. Negrito, itálico, estilos, notas e links sempre permanecem. Um instantâneo de cada documento é feito antes (inspetor, aba de instantâneos), para que você possa voltar.

### Estilos

Um estilo guarda uma formatação sob um nome. O texto lembra o nome, de modo que, quando você altera o estilo, todo o texto que o usa muda junto, no projeto inteiro.

Há dois tipos:

- **Estilos de parágrafo** aplicam-se a parágrafos inteiros. Eles contêm formatação de parágrafo (alinhamento, recuos, espaçamento, espaçamento entre linhas, paradas de tabulação) e, se você quiser, formatação de caractere para o parágrafo (fonte, tamanho, negrito, itálico, cor). Nas listas, eles são marcados com "¶", ou "¶a" quando incluem formatação de caractere.
- **Estilos de caractere** aplicam-se ao texto selecionado, ou à palavra no cursor. Eles contêm apenas formatação de caractere. Eles são marcados com "a".

Deixe o texto do corpo em **Sem estilo**. Ele segue a formatação padrão do projeto, e a Compilação pode dar a ele qualquer aparência. Use estilos apenas para o texto que deve ter aparência diferente do texto do corpo.

**Como aplicar um estilo**: escolha-o na lista à esquerda da barra de formatação, em **Formatar > Estilo**, no menu de clique com o botão direito no texto ou no **Painel de estilos** (**Formatar > Estilo > Painel de estilos**), uma pequena janela que permanece aberta ao lado do texto; um clique duplo em uma linha aplica o estilo. **Sem estilo** remove um estilo de caractere da seleção; onde não há nenhum, remove o estilo de parágrafo. As janelas e perguntas que o Painel de estilos abre aparecem acima do painel.

**A lista de estilos** da barra de formatação e do menu de clique com o botão direito (**Estilo**) mostra os estilos que você escolhe, na ordem que você escolhe: **Personalizar lista de estilos…** (na parte de baixo da lista e em **Formatar > Estilo**) lista todas as entradas; desmarque as que você não precisa, arraste-as para outra ordem ou use as setas, e **Todos, na ordem dos estilos** traz de volta a lista completa. Com mais de dez entradas, a lista mostra dez e rola. Abaixo dela, sempre à vista: **Novo estilo a partir da seleção…**, **Redefinir estilo a partir da seleção…**, **Painel de estilos** e **Personalizar lista de estilos…**. Um estilo que não está na lista ainda é nomeado na barra de formatação quando o texto o usa; **Formatar > Estilo** sempre lista todos os estilos. A escolha pertence ao projeto (um novo estilo aparece no fim da lista) e faz parte dos estilos nos arquivos de configurações do projeto.

**Sem estilo é o estilo do texto principal.** No Painel de estilos, clique com o botão direito em **Sem estilo** (ou selecione-o e use o botão Redefinir) e escolha **Redefinir a partir da seleção…**: a fonte, o tamanho, o espaçamento entre linhas, o recuo da primeira linha, o espaço depois dos parágrafos e o alinhamento do parágrafo no cursor tornam-se o estilo do texto principal do projeto (os valores de **Projeto > Configurações do projeto > Formatação**). Uma pergunta mostra primeiro os valores. Todo o texto sem estilo no projeto inteiro acompanha, também o texto com fonte, tamanho ou espaçamento próprio; o texto com estilo mantém o que o seu estilo define. Todo documento que muda recebe antes um instantâneo.

**Como criar um estilo**:

1. Formate um texto do jeito que o estilo deve ficar, e clique dentro dele ou selecione parte dele.
2. Escolha **Formatar > Estilo > Novo estilo a partir da seleção…** (ou o botão de mais do Painel de estilos).
3. Digite um nome e escolha o que guardar: toda a formatação (um estilo de parágrafo com formatação de caractere), apenas a formatação de parágrafo ou os atributos de caractere (um estilo de caractere).
4. Decida se a fonte e o tamanho da fonte fazem parte do estilo. Sem eles, o estilo mantém a fonte do texto ao redor.
5. Para estilos de parágrafo, escolha qual estilo o próximo parágrafo recebe quando você pressiona Enter no fim.

Um estilo de parágrafo guarda apenas o que difere do texto do corpo. Todo o resto continua seguindo a formatação do projeto, de modo que uma nova fonte padrão chega também ao texto com estilo.

**Como alterar um estilo**:

- **Formatar > Estilo > Redefinir estilo a partir da seleção…** toma a formatação do texto no cursor, como acima. Um estilo de parágrafo continua sendo de parágrafo, um estilo de caractere, de caractere.
- **Formatar > Estilo > Editar estilo** (ou o botão de lápis do Painel de estilos) abre uma janela com o nome, **Fonte…**, **Parágrafo…** e o estilo do parágrafo seguinte. Uma pré-visualização mostra o resultado.

Depois, o {{APP}} percorre o projeto inteiro e atualiza o texto, nos documentos abertos e nos arquivos dos demais. O que a alteração toca (fonte, tamanho, negrito, itálico, cor, espaçamento entre letras, maiúsculas, alinhamento, recuos, espaçamento, espaçamento entre linhas, paradas de tabulação) alcança **todo** o texto do estilo, também onde esse texto tinha um valor próprio: definido manualmente ou trazido por uma importação (os títulos importados costumam nomear a sua fonte, o seu tamanho e o seu espaçamento em cada parágrafo). A formatação que a alteração não toca permanece como está, por exemplo uma palavra colorida manualmente em um título quando só o tamanho do título mudou. Todo documento que muda recebe antes um instantâneo ("Before the style ... changed"). Isso não pode ser desfeito com **Editar > Desfazer**; restaure um instantâneo, redefina o estilo de novo ou use **Redefinir para a definição interna** no Painel de estilos para um estilo interno.

**Como renomear e excluir**: renomeie um estilo em **Editar estilo**. Os títulos, Título, Subtítulo, Citação em bloco, Centralizado e Legenda mantêm os seus nomes, porque a Compilação e os arquivos Markdown os conhecem pelo nome. Quando você exclui um estilo (**Formatar > Estilo > Excluir estilo**), o texto que o usava mantém a sua aparência como formatação definida manualmente.

**Outros projetos**: **Formatar > Estilo > Importar estilos…** lê os estilos de outro projeto. **Usar estes estilos em novos projetos** torna os estilos do projeto aberto o conjunto inicial de todo novo projeto; **Arquivo > Opções > Edição** volta aos estilos internos.

No Painel de estilos, os botões de seta alteram a ordem dos estilos em todas as listas.

### Markdown ou RTF: como os documentos são armazenados

Cada projeto armazena os seus documentos em um de dois formatos. Você escolhe quando cria ou importa um projeto, e pode trocar depois em **Projeto > Configurações do projeto > Formatação**.

- **Markdown** (recomendado): arquivos de texto simples que qualquer programa pode abrir, agora e daqui a cinquenta anos. A aparência do texto vem da formatação padrão. Fontes e formatação de parágrafo que você define manualmente são gravadas no texto como pequenos marcadores, somente onde você as usou (seção 18); todo o resto permanece simples.
- **RTF**: arquivos de texto rico como um processador de texto. Cada parágrafo carrega a sua formatação completa, de modo que uma mudança na formatação padrão não alcança o texto existente.

A troca converte todos os documentos. De RTF para Markdown, fontes, cores, alinhamento, recuos à esquerda e à direita e paradas de tabulação são mantidos. O espaçamento entre linhas, o recuo da primeira linha e o espaçamento entre parágrafos de texto vindo de outros programas passam a acompanhar o projeto depois. Faça um backup antes (**Arquivo > Backup > Fazer backup agora**).

O texto colado de outros programas em um projeto Markdown assume a aparência do texto ao redor (negrito, itálico e links permanecem). O texto copiado dentro do {{APP}} mantém as suas fontes. **Editar > Colar com formatação** (**Ctrl+Alt+Shift+V**) mantém também a aparência do texto de um processador de texto ou de uma página da web: fontes, tamanhos, cores e o alinhamento, os recuos e o espaçamento dos seus parágrafos (o primeiro parágrafo colado traz a sua formatação de parágrafo apenas para um parágrafo vazio). **Editar > Colar e corresponder ao estilo** (**Ctrl+Shift+V**) faz o contrário e cola texto simples. O menu de clique com o botão direito do texto oferece os três, um abaixo do outro: **Colar**, **Colar e corresponder ao estilo** e **Colar com formatação**.

Para converter um único arquivo fora de um projeto, use **Arquivo > Converter arquivo Markdown ⇄ RTF…**.

### Comentários e notas de rodapé

- **Comentário** (**Shift+F2**): selecione um texto e adicione um comentário. O texto é realçado e o comentário aparece no inspetor.
- **Nota de rodapé** (**Ctrl+Shift+F2**): selecione a palavra a que a nota de rodapé pertence; escreva a nota de rodapé no inspetor.
- **Anotação no texto** (**Ctrl+Shift+A**): uma nota escrita diretamente no texto, mostrada em cor dentro de um balão. As anotações são removidas quando você compila (a menos que você escolha exportá-las como comentários).
- **Nota de rodapé no texto** (**Ctrl+Alt+F**): uma nota de rodapé escrita diretamente no texto.

Para alterar a cor de uma anotação no texto, selecione-a (**Editar > Selecionar > Selecionar anotação ou nota** seleciona a nota inteira no cursor) e escolha uma cor de texto; a anotação mantém essa cor. **Formatar > Fonte > Cor de texto automática** devolve a ela o vermelho habitual.

Clique em um comentário no inspetor para saltar até ele. Clique com o botão direito nele para alterar a sua cor, convertê-lo em outro tipo de nota ou excluí-lo.

Um clique em um texto comentado abre o inspetor com esse comentário. **Alt+clique** abre o comentário ou a nota de rodapé em uma pequena janela no mouse, onde você pode lê-lo e alterá-lo sem o inspetor; clique em outro lugar ou pressione **Esc** para fechar.

### Imagens e links

- **Inserir > Imagem de um arquivo…** coloca uma imagem no texto; você também pode arrastar uma imagem para o texto ou colar uma. Clique com o botão direito na imagem e escolha **Tamanho da imagem…** para alterar o tamanho com que ela aparece.
- **Inserir > Link para documento…** (**Ctrl+L**) vincula o texto selecionado a outro documento do projeto. Clicar no link abre esse documento (no outro editor, no mesmo editor, no seu suporte de cópia ou em um painel de Referência rápida; veja **Arquivo > Opções > Comportamentos**).
- **Inserir > Inserir link da web…** vincula a uma página da web.
- Os links da web e de e-mail (*http*, *https*, *mailto*) abrem de imediato. Qualquer outro destino de um link, em um texto, um mapa mental, um favorito ou uma página da web importada (um arquivo, uma pasta, um endereço de rede, o tipo de link de outro programa), é mostrado por inteiro primeiro e aberto somente quando você clica em **Abrir**; esses links podem vir com um projeto, uma página da web ou um arquivo do Word de outra pessoa. Para um arquivo ou uma pasta no seu computador, a pergunta nomeia o seu tipo (por exemplo "documento de texto simples (.txt)"), o seu nome completo e a sua pasta; um nome longo quebra a linha, de modo que o seu final pode ser lido, e um arquivo que não existe é declarado ausente. Os links para programas, scripts, atalhos e instaladores nunca são abertos; abra-os você mesmo pelo seu gerenciador de arquivos, se confiar neles.

### Tabelas

**Inserir > Tabela…** pede o número de linhas e de colunas e o design, e coloca a tabela no cursor. Dentro de uma tabela, **Tab** vai para a próxima célula (na última célula, adiciona uma linha) e **Shift+Tab** volta. **Formatar > Tabela**, também no menu de clique com o botão direito dentro de uma tabela, tem o resto:

- **Inserir linha acima / abaixo**, **Inserir coluna à esquerda / à direita**.
- **Excluir linhas** e **Excluir colunas** (as das células selecionadas, ou da célula com o cursor), **Excluir tabela**.
- **Mesclar células** junta as células selecionadas em uma; **Dividir célula** divide de novo uma célula mesclada.
- **Cor da célula…** dá às células selecionadas um fundo próprio; **Cor da célula do design** o retira de novo.
- **Propriedades da tabela…** altera o design.

**O design.** Cinco designs vêm prontos: **Grade**, **Grade com linha de cabeçalho**, **Linhas alternadas**, **Linhas entre as linhas** e **Sem linhas**. Escolha um e altere o que quiser; o design passa então a ser **Personalizado**:

- **Linhas**: largura (0: sem linhas), cor e **Somente entre as linhas** (sem linhas nas laterais).
- **Células**: o espaço dentro de uma célula e o espaço entre células (0: células vizinhas compartilham uma linha).
- **Linhas de cabeçalho**: quantas linhas no alto são títulos, com a sua cor. A impressão e a compilação as repetem no alto de cada página para a qual uma tabela longa continua. **Uma linha sim, outra não** recebe uma cor própria se você escolher uma.
- **Lugar no texto**: a largura (uma fração da largura do texto, ou tão larga quanto o seu conteúdo), as larguras das colunas (todas iguais, ou frações como "30, 70"), o alinhamento e onde ela fica: **Em linhas próprias**, ou **Esquerda** ou **Direita** com o texto fluindo ao lado dela.

As linhas e colunas que você adiciona depois mantêm o design (as listras continuam). O texto nas células de tabela não é justificado, também quando o texto do corpo é (uma célula estreita espalharia as suas palavras); um alinhamento que você dá a um parágrafo em uma célula permanece. As tabelas são mantidas nos dois formatos de armazenamento e no formato do Scrivener; as tabelas em arquivos RTF de outros programas (por exemplo um arquivo do Word salvo como RTF) também se tornam tabelas.

### Imagens ao lado do texto, legendas e descrições

Clique com o botão direito em uma imagem e escolha **Imagem…** (ou ponha o cursor ao lado dela e escolha **Formatar > Imagem…**):

- **Largura na tela**: a altura acompanha; **Tamanho original** volta ao tamanho próprio da imagem.
- **Posição**: **Na linha, como uma letra** (como antes), **Esquerda** ou **Direita** com o texto fluindo ao redor, ou **Centralizada em linhas próprias**.
- **Legenda**: mostrada abaixo da imagem e compilada com ela. Uma imagem na linha não tem legenda; dê a ela primeiro uma posição.
- **Descrição**: o que a imagem mostra, em uma ou duas frases. Ela não é mostrada no texto. E-books, arquivos do Word e Markdown a guardam com a imagem (como o seu "texto alternativo"), e os leitores de tela a leem para pessoas que não podem ver a imagem.

A imagem com a sua legenda se move como uma só peça; digite a legenda abaixo da imagem como qualquer outro texto (ela tem o estilo **Legenda**).

### Localizar e substituir

- **Ctrl+F** (**Editar > Localizar > Localizar**) abre a barra de busca acima do texto. Ela mostra quantas ocorrências há e as realça; **F3** e **Shift+F3** vão para a ocorrência seguinte e a anterior. **Diferenciar maiúsculas e minúsculas** e **Palavras inteiras** restringem a busca.
- **Ctrl+H** adiciona a linha de substituição: **Substituir** altera a ocorrência selecionada e vai para a próxima, **Substituir tudo** altera todas as ocorrências no texto que você vê (um passo para **Desfazer**). O novo texto assume a formatação do texto que ele substitui, e os comentários continuam ligados.
- **Ctrl+E** (**Usar seleção para localizar**) coloca o texto selecionado na barra de busca.
- No Texto combinado, localizar e substituir funcionam em todos os documentos mostrados.
- Enquanto o teclado está em um **suporte de cópia**, Ctrl+F busca no documento do suporte de cópia.
- **Ctrl+Shift+F** busca no projeto inteiro, **Editar > Localizar > Substituir no projeto** substitui no projeto inteiro, e **Editar > Localizar > Localizar por formatação** procura realces, comentários, cores e estilos (tudo na seção 9).
- No quadro de cortiça e na estrutura de tópicos, Ctrl+F abre em vez disso a **barra de filtro** (seção 5).

### Ortografia

As palavras com erro de ortografia recebem um sublinhado ondulado vermelho; clique com o botão direito em qualquer ponto da palavra sublinhada para ver sugestões ou para adicionar a palavra ao seu dicionário (também em palavras com apóstrofo, como "Kayel's" ou "D'Nersun": o menu sempre oferece a palavra inteira que está sublinhada). Em uma palavra com erro, o menu é curto, para que abra bem junto ao mouse: as sugestões, **Adicionar "..." ao dicionário**, **Ignorar "..."** e **Mais comandos** com tudo o que o menu contém em outros lugares (Recortar, Copiar, Colar, Estilo, comentários, Aparência ...). O nome do próprio programa e os seus termos ({{APP}}, outliner, subdocument, copyholder) sempre contam como corretos. Escolha o idioma em **Arquivo > Opções > Correções** (página Ortografia); os dicionários que você instalou como complementos (um para cada idioma do programa, cada um com a sua licença) estão nessa lista; ali você também escolhe o sublinhado (ondas, pontos ou uma linha reta), se as palavras com números ("Route66") são verificadas, e **Ver lista pessoal de palavras...** mostra as palavras que você adicionou e remove as que você adicionou por engano. Os dicionários vêm do seu sistema e de complementos. No Ubuntu e no Kubuntu, o inglês é instalado; para outros idiomas, instale o pacote *hunspell* correspondente no Discover (por exemplo "hunspell-pt-br" para o português do Brasil).

### Inserções especiais

O menu **Inserir** também oferece a data e a hora atuais, caracteres especiais (travessão, meia-risca, reticências, espaço não separável, sinal de seção, marcador), **numeração automática** e **espaços reservados**, que a Compilação substitui (seção 13).

### Correções durante a digitação

**Arquivo > Opções > Correções** define o que é corrigido enquanto você digita. Uma palavra é corrigida quando você a termina, isto é, quando digita um espaço, um Return ou um sinal de pontuação depois dela; **Ctrl+Z** logo em seguida desfaz apenas a correção.

- **Corrigir maiúsculas no início das frases**: a primeira palavra de uma frase recebe uma letra maiúscula (não depois de abreviaturas como "Dr." ou "e.g."). **Escrever "i" em maiúscula**: "i" e "i'm" viram "I" e "I'm". Os dois estão desativados no início.
- **Aspas inteligentes** no estilo que você escolher (“inglês”, „alemão“, »alemão«, «francês», ”sueco”); um apóstrofo dentro de uma palavra é sempre ’. Onde a marca simples de fechamento não é o apóstrofo (alemão ‚…‘, francês ‹…›), um ' digitado logo depois de uma palavra fecha uma marca simples que esteja aberta no parágrafo; se você continuar digitando letras ("geht's"), ele volta a ser o apóstrofo. Dois hifens viram um travessão (—); três pontos podem virar um único caractere de reticências (desativado no início). No modo de roteiro, essas correções podem ser desativadas.
- **Aspas que já estão no projeto** (desde 0.61.1): em **Aspas inteligentes**, **Converter em aspas tipográficas…** transforma os " e ' retos do projeto nas marcas do estilo escolhido ao lado de **Usar aspas inteligentes**, e **Converter em aspas retas…** transforma toda aspa tipográfica e todo apóstrofo de volta em " e '. Uma janela como a de Substituir no projeto (seção 9) abre: a direção e o estilo mais uma vez, onde converter (títulos, texto, notas, sinopses, metadados personalizados, comentários e notas de rodapé), o escopo (o projeto inteiro ou os documentos selecionados no Fichário, a Lixeira somente quando você a marca) e um instantâneo de cada texto antes de ele mudar (marcado no início). **Contar** mostra quantas marcas mudariam; **Converter** pergunta mais uma vez e faz a conversão. A formatação permanece (uma aspa em negrito continua em negrito). Uma marca abre depois de nada, de um espaço, de um parêntese, de um travessão ou de uma marca de abertura, e fecha nos demais casos; um apóstrofo dentro de uma palavra ou antes de um número ('90s) vira ’. Uma marca reta com um espaço dos dois lados permanece como está, e um apóstrofo no início de uma palavra ('tis) vira uma marca de abertura: confira esses casos depois. Localizar e substituir não consegue fazer isso, porque uma mesma marca precisa virar de abertura ou de fechamento conforme os vizinhos.
- **Detecção de dados**: um endereço da web digitado vira um link; **[[Título]]** vira um link para o documento com esse título, e se não houver nenhum, um novo documento vazio com esse nome é criado ao lado do texto (os "links de documento" do Scrivener).
- **Substituições adicionais**: uma lista do que você digita e no que isso se transforma, por exemplo "(c)" e ©, "->" e →. **Editar substituições...** altera a lista; **Substituições padrão** traz de volta a lista com que o programa veio.
- **Preenchimento automático**: no modo de roteiro, a lista de nomes de personagens e cabeçalhos de cena oferecida enquanto você digita.

### Zoom, rolagem de máquina de escrever e largura fixa

- **Ctrl+roda do mouse** dá zoom na área sob o mouse, onde quer que ela esteja: os editores, o Fichário, o inspetor, a estrutura de tópicos, o quadro de cortiça (as fichas e o texto delas juntos), um mapa mental, o modo de composição e janelas próprias (Referência rápida, as ferramentas de escrita); o teclado pode estar em outro lugar. Cada área mantém o seu zoom até você alterá-lo de novo.
- **Ctrl++ / Ctrl+-** ou a caixa de zoom no rodapé aumentam o texto na tela sem alterar o tamanho da fonte. **Visualizar > Redefinir zoom** (**Ctrl+0**) volta ao zoom padrão do texto.
- **Arquivo > Opções > Edição** define o zoom com que cada área começa ("Zoom padrão do texto", para cada área da janela e cada parte com zoom próprio: a sinopse, as notas, as listas de favoritos, o texto dos favoritos, a lista e o texto dos instantâneos, a lista e a ficha de objetos no inspetor e na janela de Objetos, novos mapas mentais) e se os editores reabrem com o zoom que tinham por último ("Recarregar o último zoom selecionado nos editores principais").
- **Onde você estava.** O {{APP}} lembra, junto com o projeto, o lugar do cursor e a parte do texto na tela de cada documento: mostrado de novo (pelo Fichário, por um link, por Voltar e Avançar, no suporte de cópia, em um painel de Referência rápida ou depois de reabrir o projeto), um documento abre onde você o deixou. Uma pasta no Texto combinado lembra qual dos seus documentos tinha o cursor. O resultado de uma busca ou um link que aponta para um certo lugar vai para lá. As imagens mantêm o seu zoom e os arquivos PDF o seu zoom e a sua página.
- **Rolagem de máquina de escrever** (**Visualizar > Edição de texto**) mantém a linha em que você está digitando no meio da tela.
- O **editor de largura fixa** mostra o texto como uma coluna centralizada de largura confortável (500 px a 100 %, a menos que você a altere), por mais larga que a janela seja. Desative-o, ou altere a largura, em **Arquivo > Opções > Aparência > Editor principal** ("Usar editor de largura fixa", "Largura da coluna de texto"), ou com um clique com o botão direito no papel (**Aparência > Largura da coluna de texto**). O papel da coluna é #d1d495 e a área ao lado dela #999999 por padrão ("Papel" e "Ao lado da coluna de texto" entre as cores do Editor principal); elas permanecem iguais quando você escolhe outro tema, até você alterá-las ou redefini-las.
- **Borda ao lado do texto.** À esquerda e à direita do texto, o papel tem uma borda com cor própria, para que o texto não encoste na margem do papel: 10 px a 100 % para novos projetos, e ela cresce com o zoom. Ela fica fora da coluna de texto, de modo que as linhas quebram onde quebravam. Altere em **Arquivo > Opções > Aparência > Editor principal** ("Borda ao lado do texto", de 0 a 100 px) ou com um clique com o botão direito no papel (**Aparência > Borda ao lado do texto**). Os projetos feitos antes de 0.40.2 mantêm o visual que tinham, sem borda, até você dar uma a eles. Um clique na borda coloca o cursor no início ou no fim da linha ao lado dela.
- **Clicar e clicar com o botão direito** nunca movem a visão: o cursor vai para onde você clica, e o texto permanece onde está na tela (antes de 0.40.2, um clique ou um clique com o botão direito perto da borda de cima ou de baixo, ou um clique com o botão direito ao lado do texto, podia fazer o texto saltar).

### Visualização de página

**Visualizar > Edição de texto > Visualização de página** (também no menu do botão **Visualizar** da barra de ferramentas) mostra o seu texto em folhas de papel em vez de uma coluna sem fim. O rodapé mostra então a página do cursor e o número de páginas, por exemplo "Página 3 de 12". Escolha o comando de novo para voltar à coluna. A cor da mesa ao redor das páginas está em **Arquivo > Opções > Aparência > Editor principal** (ou clique com o botão direito > **Aparência** enquanto as páginas são mostradas).

As páginas são uma forma de olhar para o seu texto, nada mais. Elas não mostram como será um manuscrito compilado ou impresso: você escreve em trechos, e títulos, fontes e tamanho da página são decididos quando você compila (seção 13). Nada sobre as páginas é guardado nos seus documentos.

- **Visualizar > Edição de texto > Tamanho da página e margens…** define o papel (A4, A5, Carta dos EUA e outros, ou um tamanho próprio), retrato ou paisagem, e as quatro margens. A configuração pertence ao projeto; marque **Usar esta página também para novos projetos** para torná-la o seu padrão.
- O zoom aumenta folhas, margens e letras juntas. Uma folha mais larga que o editor pode ser rolada para os lados. Como na visualização em coluna, recuos e paradas de tabulação mantêm o seu comprimento real, de modo que as quebras de linha podem diferir um pouco entre os níveis de zoom.
- No Texto combinado, cada documento começa em uma folha própria, e a última folha dele termina logo abaixo do texto, de modo que documentos curtos não enchem a tela de papel vazio.
- A visualização de página vale para tudo o que mostra os seus textos na janela principal e nas janelas de referência rápida: os dois editores e os suportes de cópia. As notas mantêm o seu visual simples, e o modo de composição tem o seu próprio papel.
- A cor ao redor das folhas é **Ao redor das páginas (visualização de página)** em **Arquivo > Opções > Aparência > Editor principal**.

### Roteirização

Roteiros de cinema, peças de teatro e roteiros de quadrinhos são escritos no **modo de roteiro**. Um roteiro é feito de *elementos*: cabeçalho de cena, ação, personagem, rubrica, diálogo, transição e assim por diante. Cada elemento tem o seu lugar na página, e no modo de roteiro as teclas **Tab** e **Enter** levam você de um elemento para o seguinte, de modo que você nunca define um recuo manualmente.

**Como ativá-lo.** Clique em um documento e escolha **Formatar > Roteirização > Modo de roteiro** (**Ctrl+8**). A entrada do menu nomeia o formato de roteiro do projeto, por exemplo *Modo de roteiro - Roteiro de cinema*. O modo de roteiro pertence ao documento: roteiros e textos comuns convivem lado a lado em um mesmo projeto, e os roteiros têm um **ícone amarelo** no Fichário. Escolha o comando de novo para desativá-lo; o texto mantém a sua aparência. Um novo documento criado entre roteiros (em uma pasta cujos textos são todos roteiros) também é um roteiro.

**O rodapé.** Em um roteiro, o rodapé do editor mostra, no lugar da contagem de palavras, o que **Tab** e **Enter** farão no cursor, e à direita um menu suspenso com o elemento do parágrafo atual. Escolha outro elemento nesse menu e o parágrafo é reformatado. O texto que não é um elemento do roteiro é mostrado como **Texto geral**.

**Tab e Enter.**

- **Enter** no fim de um parágrafo inicia o elemento que normalmente vem em seguida: ação depois de um cabeçalho de cena, diálogo depois do nome de um personagem e assim por diante.
- **Enter** em uma linha vazia abre a lista de elementos no cursor; pressione a letra que está sublinhada na lista (a de Cabeçalho de cena, a de Personagem ...). **Formatar > Roteirização > Mostrar a lista de elementos** (**Ctrl+\\**) abre a mesma lista a qualquer momento.
- **Tab** em uma linha vazia muda a linha para outro elemento; pressione-o de novo para o seguinte, **Shift+Tab** para voltar.
- **Tab** no fim de um parágrafo faz o que convém ao elemento: depois do nome de um personagem ou de uma linha de diálogo, ele inicia uma rubrica, e em um cabeçalho de cena digita " - ", pronto para a hora do dia.
- **Enter** no meio de um parágrafo o divide; as duas metades mantêm o elemento. **Shift+Enter** é uma quebra de linha dentro do parágrafo.
- Os elementos escritos em maiúsculas (cabeçalhos de cena, nomes de personagens, transições) transformam em maiúsculas o que você digita. Parênteses e prefixos como "SFX: " são digitados para você.

**Preenchimento automático.** Enquanto você digita, uma pequena lista oferece o que combina com o elemento: INT. e EXT. no início de um cabeçalho de cena, as horas do dia depois do seu " - ", os lugares e nomes de personagens já usados no documento e transições padrão como CUT TO:. **Enter** ou **Tab** aceita a entrada marcada, as setas escolhem outra, **Esc** fecha a lista.

**Como mudar elementos.** **Formatar > Roteirização > Alterar elemento para** altera o parágrafo com o cursor, ou todos os parágrafos selecionados. Escolher um elemento ali em um documento comum transforma o documento em um roteiro. Todo elemento é um comando, de modo que você pode dar um atalho aos seus favoritos em **Arquivo > Opções > Teclado**.

**Formatos de roteiro.** Um projeto usa um formato de roteiro para todos os seus roteiros. **Formatar > Roteirização > Formato de roteiro** lista *Roteiro de cinema*, *Peça de teatro* e *Roteiro de quadrinhos*, os formatos de complementos e os seus próprios. Quando você escolhe outro formato, os roteiros do projeto recebem o novo layout: cada elemento se torna o elemento do mesmo tipo no novo formato (a ação vira uma rubrica de cena, por exemplo). Um elemento sem correspondente vira texto geral que mantém a sua aparência; a pergunta antes da alteração nomeia esses elementos.

**Configurações de roteiro.** **Formatar > Roteirização > Configurações de roteiro…** altera o formato do projeto ou cria um novo:

- o nome do formato, a sua fonte (por padrão a fonte de máquina de escrever do seu sistema: Courier Prime, Courier New ou uma fonte semelhante) e o tamanho, e se os comprimentos são em polegadas ou centímetros;
- a lista de elementos: adicione, remova e reordene (a ordem é a ordem da lista de elementos e de **Tab** em uma linha vazia);
- para cada elemento: nome, tipo (o que ele é para outros programas: cabeçalho de cena, ação, personagem ...) e a sua letra na lista; negrito, itálico, sublinhado, maiúsculas e cor; alinhamento, recuos a partir das duas margens, linhas vazias antes dele e espaçamento entre linhas; o que **Enter** e **Tab** fazem; um texto que é digitado antes e depois do parágrafo; e as listas para o preenchimento automático;
- uma pré-visualização de todos os elementos.

Um elemento renomeado mantém os seus parágrafos em todos os roteiros do projeto. O botão **Gerenciar** parte de outro formato, carrega ou salva um arquivo de formato (*.qbscript*) e salva o formato para uso em outros projetos.

**Bom saber.**

- As teclas e as listas funcionam da mesma forma no Texto combinado, no segundo editor, nos suportes de cópia, nos painéis de Referência rápida e no modo de composição. Apenas os editores da janela principal têm o rodapé com as dicas; em outros lugares, **Ctrl+\\** mostra a lista de elementos.
- A visualização de página (acima) mostra um roteiro em folhas; com a fonte de máquina de escrever usual em 12 pontos, uma página de um roteiro de cinema equivale a cerca de um minuto de filme, mas as páginas na tela não são as páginas do arquivo compilado.
- A fonte e os recuos dos elementos pertencem ao formato de roteiro, não ao documento. Os arquivos dos seus roteiros contêm apenas os nomes dos elementos (seção 18).
- Importar e exportar roteiros, nos formatos de outros programas de roteirização: seção 14. Compilar um roteiro inteiro: seção 13.
- Os roteiros exigem a versão 0.10 ou posterior. Um projeto que contém roteiros diz isso no seu arquivo de projeto, e a partir desta versão o {{APP}} pergunta antes de abrir um projeto que precisa de uma versão mais nova que a que está em execução. As versões 0.9 e anteriores não perguntam: elas mostram os roteiros como texto comum e perdem os elementos de todo roteiro que você olha ali.

---

## 5. Três formas de ver o seu projeto

Escolha a visualização com os três botões da barra de ferramentas ou com **Ctrl+1**, **Ctrl+2** e **Ctrl+3**.

### Texto combinado (Ctrl+1): vários documentos como um só texto

Selecione vários documentos no Fichário (Ctrl+clique ou Shift+clique) ou uma pasta, e pressione **Ctrl+1**. Todos os textos aparecem um após o outro, separados por uma fina linha tracejada, e você pode escrever em todos eles como se fossem um só documento. As alterações vão para os documentos certos automaticamente. Mostre os títulos dos documentos entre eles com **Visualizar > Edição de texto > Mostrar títulos no Texto combinado**.

### Quadro de cortiça (Ctrl+2): fichas

Cada documento é uma ficha que mostra o seu título e a sua sinopse; um documento sem sinopse mostra em vez disso as primeiras linhas do seu texto (esmaecidas, em itálico). A ficha de uma imagem mostra a imagem, e a de um documento cuja sinopse é uma imagem também (seção 7). Arraste fichas para reordenar os documentos. Dê um clique duplo em uma ficha para editar o seu título ou a sua sinopse. Pressione **Enter** para abrir as fichas selecionadas como texto.

No rodapé do quadro de cortiça:

- **Grade** ou **livre**: no modo livre você pode colocar as fichas em qualquer lugar, como em um quadro de cortiça de verdade. **Confirmar ordem** então reordena os documentos no Fichário para corresponder às fichas da esquerda para a direita, de cima para baixo. A disposição que você escolhe para uma pasta é lembrada com essa pasta (desde 0.90.0): na próxima vez que você a mostrar, o seu quadro de cortiça terá a mesma disposição. Uma pasta para a qual você nunca escolheu uma disposição mantém a que o editor mostra.
- **Fios de etiqueta**: organiza as fichas em linhas por etiqueta.
- **Tamanho das fichas**, com o controle deslizante, as setas do teclado ou a roda do mouse sobre ele; é mantido de imediato (**Ctrl+roda do mouse** sobre o quadro de cortiça dá zoom nas fichas e no texto delas juntos, como nas outras áreas, veja a seção 4).

Um clique duplo em um lugar vazio do quadro de cortiça cria uma nova ficha (**Arquivo > Opções > Comportamentos > Clique duplo**). Quando você edita uma sinopse em uma ficha, **Return** termina a edição e **Shift+Return** cria uma nova linha (**Arquivo > Opções > Comportamentos > Tecla Return**, onde Return também pode ser configurada para criar uma nova ficha ou linha da estrutura de tópicos depois da selecionada).

Os carimbos de status, o modo livre e a organização por etiqueta também estão em **Visualizar > Opções do quadro de cortiça**; as tonalidades de etiqueta para fichas e Fichário, em **Visualizar > Usar cor da etiqueta em**. As colunas da estrutura de tópicos também estão listadas em **Visualizar > Opções da estrutura de tópicos**.

### Estrutura de tópicos (Ctrl+3): uma tabela

Mostra os documentos como uma tabela com título, sinopse, etiqueta, status, contagens de palavras, metas, datas e os seus metadados personalizados. Clique no botão de colunas no rodapé para escolher as colunas. Clique no cabeçalho de uma coluna para ordenar (apenas para visualização; a ordem do Fichário não muda). Arraste linhas para reordenar os documentos.

### Como filtrar o quadro de cortiça e a estrutura de tópicos

Enquanto o quadro de cortiça ou a estrutura de tópicos tem o teclado, **Ctrl+F** (**Editar > Localizar > Localizar**) abre uma **barra de filtro** no alto do editor. Conforme você digita, ficam apenas as fichas ou linhas que correspondem; a barra diz quantas ("2 de 9"). Na estrutura de tópicos, os grupos dos documentos correspondentes permanecem para que você veja a que lugar eles pertencem.

- Clique no **triângulo** à esquerda do campo para ver as opções: onde procurar (**Títulos**, **Sinopses**, **Palavras-chave**, **Texto**), uma **etiqueta**, um **status**, uma **palavra-chave**, se o documento está **incluído na compilação** e **Mostrar o que não corresponde**. Várias escolhas juntas restringem ainda mais a lista.
- O filtro permanece enquanto você abre outras pastas, de modo que você pode percorrer o projeto olhando apenas, digamos, as cenas de um personagem.
- **Concluído** remove o filtro e mostra tudo de novo.

Arrastar fichas ainda reordena os documentos: uma ficha solta entre duas fichas que você vê cai ao lado delas no Fichário. **Confirmar ordem** do quadro de cortiça livre fica desativado enquanto um filtro oculta fichas.

---

## 6. Dois editores, favoritos e referência rápida

### Editor dividido

Clique no botão de divisão à direita do cabeçalho do editor para mostrar um segundo editor. Alt+clique divide na outra direção. **Visualizar > Dividir editor** oferece as mesmas opções. **F6** salta entre os editores. Clicar no Fichário mostra o documento no editor que você usou por último; **Navegar > A seleção do Fichário afeta** altera isso para o editor da esquerda ou de cima, o da direita ou de baixo, ou o outro editor, qualquer um em que você esteja escrevendo.

Em um quadro de cortiça ou em uma estrutura de tópicos, o botão do rodapé **A seleção afeta o outro editor** (também em **Visualizar > Opções do quadro de cortiça** e **Opções da estrutura de tópicos**) faz o outro editor mostrar a ficha em que você clica. É uma forma rápida de navegar entre cenas mantendo a visão geral do capítulo.

**Navegar > Editor > Travar no lugar** mantém um editor no seu documento, mesmo quando você clica em outro lugar do Fichário.

### Suportes de cópia

Cada editor tem um **suporte de cópia**: uma segunda visão, simples, dentro do editor, que guarda outro documento para consulta, como notas ou uma ficha de personagem. Com dois editores e os seus suportes de cópia, você pode ver quatro documentos ao mesmo tempo.

- **Para abri-lo**, mantenha **Alt** pressionada e arraste um documento do Fichário, de um quadro de cortiça ou de uma estrutura de tópicos para a barra de cabeçalho do editor. Sem Alt, o que é solto carrega o documento no próprio editor. **Navegar > Abrir > no suporte de cópia** e o menu de clique com o botão direito de um documento fazem o mesmo para o documento selecionado.
- O suporte de cópia tem uma barra de cabeçalho estreita com o título e nenhum rodapé. Ele mostra o texto de um documento (ou uma imagem ou PDF); não tem quadro de cortiça, estrutura de tópicos nem Texto combinado. Você pode escrever e formatar nele como em qualquer editor.
- **Para movê-lo**, clique com o botão direito na barra de cabeçalho dele e escolha esquerda, direita, cima ou baixo. Na primeira vez, ele abre ao lado de um editor largo e abaixo de um estreito; depois disso, onde você o colocou por último. Arraste a linha entre o editor e o suporte de cópia para alterar os tamanhos deles.
- **Para mostrar outro documento**, solte-o na barra de cabeçalho do suporte de cópia, ou arraste-o com Alt sobre a barra de cabeçalho do editor de novo.
- **Para fechá-lo**, clique no **X** à direita da barra de cabeçalho dele, ou escolha **Navegar > Editor > Fechar suporte de cópia**. O botão à esquerda do X o fecha e abre o documento em uma janela de referência rápida.

**Como navegar com um suporte de cópia.** Enquanto um editor mostra um quadro de cortiça ou uma estrutura de tópicos e tem o seu suporte de cópia aberto, o botão **A seleção afeta** no rodapé tem três posições. O primeiro clique faz a sua seleção abrir no outro editor, como de costume. O segundo clique vira a seta para que ela aponte para dentro da caixa: agora a ficha ou linha em que você clica abre no suporte de cópia. Um terceiro clique o desativa. Uma disposição útil: o manuscrito no editor da esquerda, uma estrutura de tópicos da sua pesquisa no editor da direita com o suporte de cópia abaixo, e **Navegar > A seleção do Fichário afeta > Editor da esquerda ou de cima**. O Fichário então conduz o manuscrito, e a estrutura de tópicos conduz o suporte de cópia.

O suporte de cópia, o seu lugar e o seu documento são lembrados com o projeto.

### O cabeçalho do editor

O cabeçalho acima de cada editor mostra o título do documento e contém os botões **voltar** e **avançar** (**Ctrl+[** e **Ctrl+]**), um menu das seções do documento e botões para o documento anterior e o seguinte (**Alt+Shift+Up / Down**).

- **Histórico**: cada editor lembra os documentos que mostrou. Mantenha pressionado o botão de voltar ou de avançar (ou clique nele com o botão direito) para ver uma lista e escolha um documento para ir direto a ele; **Navegar > Voltar para** e **Avançar para** mostram as mesmas listas. O histórico é mantido com o projeto.
- **O ícone** ao lado do título representa o documento. Clique nele para ver um menu (Travar no lugar, Mostrar no Fichário ...). Arraste-o como um documento no Fichário: para a barra de cabeçalho do outro editor, para o Fichário, para a aba de uma coleção, para o botão Referência rápida ou Favoritos da barra de ferramentas, ou para o texto para criar um link.
- **Documentos > Agrupar** (**Ctrl+Shift+J**, também no menu de contexto do Fichário) move os itens selecionados para uma nova pasta no lugar do primeiro; o título dela está pronto para ser digitado. Com o teclado no Fichário, **Editar > Desfazer** desfaz o passo inteiro.
- **Documentos > Converter > Ficha para objeto** (também no menu de contexto do Fichário) cria um Objeto de cada documento de ficha selecionado, por exemplo uma ficha de personagem ou de local: o título é o nome, o texto vai para **Outros**, uma imagem no texto vira a imagem. O nome da pasta (Personagens, Locais) escolhe o tipo; para outra pasta, você é consultado. O item do Fichário permanece: selecioná-lo mostra o Objeto no inspetor, e Return o abre na janela de Objetos. O documento mantém o seu texto (um projeto do Scrivener vê um documento normal); o vínculo é mantido nos dados de Objetos do próprio projeto. **Editar > Desfazer** com o teclado no Fichário desfaz isso.
- **Navegar > Ir para** lista o Fichário inteiro como um menu e carrega o documento escolhido no editor ativo. **Navegar > Grupo que contém** mostra a pasta ou o grupo a que o documento pertence, com o documento selecionado ali.
- Um **clique duplo no divisor** entre dois editores deixa os dois com o mesmo tamanho.

### Arrastar e soltar de relance

| Arraste ... | para ... | e ... |
|---|---|---|
| um documento (Fichário, coleção, quadro de cortiça, estrutura de tópicos, ícone do cabeçalho, favorito) | a barra de cabeçalho de um editor | ele é mostrado nesse editor; com **Alt**: no suporte de cópia dele |
| | o texto em um editor | um link para o documento; em um texto selecionado, esse texto se torna o link |
| | o texto, com **Alt** pressionada | o texto do documento é inserido |
| uma imagem do Fichário | o texto em um editor | a imagem é inserida |
| um documento | o botão **Referência rápida** da barra de ferramentas | cada documento abre em um painel próprio |
| | o botão **Favoritos** da barra de ferramentas | ele é adicionado aos favoritos do projeto |
| | a aba de uma coleção, uma lista de favoritos | ele é adicionado |
| | a barra de cabeçalho de um painel de Referência rápida | ele é mostrado nesse painel |
| | o gerenciador de arquivos ou a área de trabalho | ele é salvo ali como um arquivo (veja "Exportar documentos") |
| texto selecionado | o Fichário | um novo documento com esse texto; o texto permanece onde estava |
| arquivos do gerenciador de arquivos | o Fichário ou um quadro de cortiça | eles são importados |
| um link da web de um navegador | uma pasta no Fichário, o quadro de cortiça, a estrutura de tópicos | a página é importada (complemento Importação de páginas da web, seção 14) |
| um arquivo de imagem, ou uma imagem do Fichário | a ficha no inspetor (modo de imagem) | ela se torna a imagem da ficha |
| palavras-chave de **Projeto > Palavras-chave do projeto** | um documento no Fichário, uma coleção, o quadro de cortiça, a estrutura de tópicos; a lista de palavras-chave do inspetor | elas são atribuídas; com **Alt**: junto com o grupo a que pertencem |

Algumas áreas de trabalho usam Alt+arrastar para mover janelas. Se Alt+arrastar não chega ao programa, altere a tecla modificadora para mover janelas nas configurações da sua área de trabalho (KDE: Configurações do sistema, comportamento das janelas; o lugar exato difere entre as versões), ou use os comandos de menu em vez disso (**Navegar > Abrir > no suporte de cópia**, **Projeto > Palavras-chave do projeto > Atribuir aos documentos selecionados**).

### Painéis de referência rápida

Um painel de referência rápida é um documento em uma janela própria, por exemplo para manter uma ficha de personagem à vista enquanto você escreve. O texto em um painel pode ser editado e formatado como no editor (os atalhos dos menus Formatar e Inserir funcionam ali).

**Para abrir painéis:**

- **Navegar > Abrir > como referência rápida**, ou o botão Referência rápida da barra de ferramentas, para os documentos selecionados.
- Pressione **Espaço** no quadro de cortiça ou na estrutura de tópicos, ou no Fichário e em uma lista de coleção ou de resultados de busca quando **Visualizar > Teclas de adição rápida no Fichário** está ativado (veja a seção 2, as teclas do Fichário). Sem essa opção, o Espaço no Fichário faz parte da digitação de um nome para saltar até ele.
- Arraste documentos para o **botão Referência rápida** da barra de ferramentas: cada um abre em um painel próprio, e o editor mantém o seu documento.
- **Navegar > Abrir referência rápida** mostra o Fichário inteiro como um menu, de modo que você pode abrir um documento sem selecioná-lo antes.

**Em um painel:**

- O **título** na barra de cabeçalho pode ser editado (ele renomeia o documento). O **ícone** ao lado dele abre um menu (Mostrar no Fichário, Abrir no editor, adicionar aos favoritos do projeto ou remover deles) e pode ser arrastado como o próprio documento. Solte um documento na barra de cabeçalho para mostrá-lo neste painel.
- O botão no **canto superior direito** mostra a **sinopse**, as **notas**, as **palavras-chave** ou os **metadados** do documento abaixo do texto dele. O que você altera ali altera o documento.
- A **barra de rodapé** mostra a contagem de palavras e permite definir a **etiqueta** e o **status**.
- O **botão de favoritos** à esquerda da barra de rodapé abre os **favoritos do projeto** ao lado do documento. Um clique em um documento favorito o mostra no painel, de modo que um painel basta para circular entre as suas notas. Arraste favoritos para outra ordem, solte documentos neles e remova um favorito com o botão ➖ (o documento permanece no Fichário). Arquivos e links da web abrem com um clique duplo.
- O botão ➕ abaixo dos favoritos cria um **novo documento** e o marca como favorito. Na primeira vez, você é consultado sobre a pasta (por exemplo uma pasta "Notas" em Pesquisa); marque **Não perguntar de novo** para usar essa pasta daí em diante. Altere em **Projeto > Configurações do projeto > Pastas especiais**.
- O botão **Flutuar** mantém o painel acima da janela principal; **Janela > Manter painéis de referência rápida à frente** faz isso com todos eles.

O menu Janela lista os painéis abertos e fecha todos de uma vez. Os painéis que estão abertos quando você fecha um projeto voltam quando você o abre de novo, com o seu lugar, tamanho, painel inferior e barra lateral (desative em **Arquivo > Opções > Comportamentos**).

### Áudio e vídeo

Os arquivos de áudio e vídeo no Fichário (por exemplo entrevistas na pasta Pesquisa) são reproduzidos dentro do {{APP}} quando o complemento **Reprodução de mídia** está instalado (seção 17; ele veio com a atualização 0.12). Sem ele, esse arquivo mostra **Abrir com o aplicativo padrão**, como antes.

**Reprodução.** Clique no arquivo no Fichário. Um vídeo mostra a sua imagem; um arquivo de áudio mostra o seu título, o artista e a duração. A **barra de rodapé** do editor tem os controles de reprodução:

- o botão de **pausar e retroceder** à esquerda (veja abaixo),
- **ir para o início**, **voltar**, **reproduzir / pausar**, **avançar**,
- a **posição**: arraste o controle deslizante, ou clique ao lado da sua alça,
- o tempo, a **velocidade** (0,5× a 2×; uma fala mais lenta é mais fácil de digitar; a voz fica mais grave ou mais aguda com a velocidade, como em uma fita) e o **volume** (clique: silenciar; a seta: um controle deslizante).

Um clique na imagem de um vídeo o reproduz ou pausa; um clique com o botão direito oferece também **Abrir com o aplicativo padrão**. Com o teclado no player: **Espaço** reproduz ou pausa, as **setas para a esquerda** e **para a direita** avançam e retrocedem, **Home** vai para o início.

**Transcrição.** Divida o editor, mostre o arquivo de áudio em um editor e a sua transcrição no outro, e continue digitando: os comandos de **Navegar > Mídia** funcionam onde quer que o teclado esteja, também nos painéis de Referência rápida e no modo de composição.

| Comando | Atalho |
|---------|--------|
| Reproduzir / Pausar | Ctrl+Return |
| Voltar / Avançar (5 segundos) | Ctrl+Alt+, / Ctrl+Alt+. |
| Inserir marca de tempo de mídia (também **Inserir > Inserir marca de tempo de mídia**: o mesmo comando, mostrado nos dois menus) | Ctrl+Shift+Return |
| Parar, Ir para o início, Pausar e retroceder, Mais rápido, Mais lento, Velocidade normal, Aumentar volume, Diminuir volume, Silenciar, Ir para a marca de tempo no cursor | sem atalho (dê um a eles em **Arquivo > Opções > Teclado**) |

Os comandos agem sobre o arquivo que você abriu ou usou por último (em qualquer um dos editores, em um suporte de cópia ou em um painel de Referência rápida).

**Pausar e retroceder.** Ativado (o botão parece pressionado), cada pausa volta um pouco, 2 segundos a menos que você defina outro valor em **Arquivo > Opções > Comportamentos > Reprodução**. Quando você pressiona Ctrl+Return para digitar o que ouviu, você ouve de novo as últimas palavras quando continua.

**Marcas de tempo.** **Inserir marca de tempo de mídia** digita o tempo do player no cursor, como `[03:07]` (com horas a partir de uma hora: `[1:02:03]`). **Ir para a marca de tempo no cursor** faz o contrário: ela toma a marca de tempo na linha do cursor (a mais próxima antes dele ou ao redor dele) e leva o player até lá. As marcas de tempo são texto comum; também podem ser escritas à mão (`3:07`, `[03:07]`, `1:02:03.5`).

**Em um suporte de cópia ou em um painel de Referência rápida** o player tem os seus controles de reprodução na parte de baixo. Um arquivo de áudio no suporte de cópia do editor da transcrição é uma boa disposição para um único editor.

**Onde ficou.** Quando você abre de novo um arquivo de áudio ou vídeo, ele continua de onde você o pausou (desative em **Arquivo > Opções > Comportamentos > Reprodução**). Mostrar outro documento no editor para o arquivo.

**O que é reproduzido.** O complemento *Reprodução de mídia* (2.0) reproduz formatos de uso livre: áudio MP3, Ogg Vorbis, Opus, FLAC, ALAC, WAV, AIFF e WavPack; vídeo WebM, MKV e Ogg com imagens VP8, VP9 ou Theora (também MJPEG e outros formatos livres). O complemento traz tudo de que precisa.

**Mais formatos.** H.264, H.265 (HEVC), AAC (M4A), AC-3, E-AC-3, DTS, vídeo MPEG-1/2/4, VC-1 e WMV, WMA e AMR são licenciados por patentes em alguns países; eles vêm com o complemento separado **Formatos extras de vídeo e áudio**. O jeito mais simples de obter os dois é o pacote único **media-all-formats.qbpkg** na página Releases: instalado como todo complemento (seção 17), ele instala *Reprodução de mídia* e *Formatos extras de vídeo e áudio* juntos. Os dois aparecem então em **Ajuda > Complementos e atualizações > Gerenciador de complementos**, onde os formatos extras podem ser desativados ou removidos separadamente. Se é preciso uma licença de patente para usar esses formatos depende do país. Enquanto ele está instalado e ativado, *Reprodução de mídia* o usa, e os arquivos MP4/M4V/MOV, AVI, MPEG, WMV, FLV e 3GP também são reproduzidos (não WMA Pro e WMV 9). Sem ele, esse arquivo mostra uma mensagem abaixo do player que nomeia os seus formatos e este complemento; todo o resto funciona como antes. O vídeo AV1 não é reproduzido com a decodificação interna.

Desde 0.80.0, o player sempre decodifica com o seu próprio FFmpeg: o Qt que o {{APP}} usa desde então (a compilação própria da The Qt Company do Qt 6.12) não tem como reproduzir pelo **GStreamer** do sistema, de modo que **Arquivo > Opções > Comportamentos > Reprodução > Decodificação** mostra essa opção como indisponível.

### Favoritos

Os **favoritos de documento** (inspetor, aba de favoritos) vinculam um documento a outros documentos, a arquivos no seu disco ou a páginas da web. Os **favoritos do projeto** estão disponíveis em todos os documentos, pelo botão de favoritos na barra de ferramentas e pela barra lateral dos painéis de Referência rápida. A aba de favoritos do inspetor mostra as duas listas, uma acima da outra: **Favoritos do documento** e **Favoritos do projeto**, cada uma com os seus próprios botões ➕ e ➖. Um clique em um favorito mostra o documento abaixo das listas; um documento que o editor já mostra aparece ali como uma cópia somente para leitura (escreva no editor), para que o editor mantenha a sua largura de linha. Um clique duplo abre o favorito. Arraste documentos, arquivos ou links da web para a lista de favoritos, ou use o botão ➕; um link arrastado de um navegador da web mantém o título da página. Arraste um favorito para cima ou para baixo para alterar a ordem. Os divisores entre as duas listas e o texto abaixo delas podem ser movidos; os tamanhos que você lhes dá são guardados com o projeto e voltam depois de reiniciar ou de atualizar o programa, também quando nenhum favorito está escolhido naquele momento. Os documentos soltos no **botão Favoritos** da barra de ferramentas tornam-se favoritos do projeto; o mesmo vale para os documentos selecionados com **Documentos > Adicionar aos favoritos do projeto**; o inspetor mostra então a sua aba de favoritos com a nova entrada selecionada.

---

## 7. O inspetor

Os botões no alto do inspetor alternam entre:

- **Sinopse e notas**: a sinopse é um breve resumo que aparece na ficha. Você pode usar uma imagem em vez de texto: clique no botão de imagem da ficha e solte um arquivo de imagem ou uma imagem do Fichário na área cinza (ou dê um clique duplo na área para escolher um arquivo; clique com o botão direito para remover a imagem). Abaixo dela estão as **notas do documento** (ou, alternadas pelo pequeno menu, as **notas do projeto**, compartilhadas por todo o projeto). **Navegar > Inspecionar > Ir para a sinopse** (**Ctrl+Alt+Y**) e **Ir para as notas do documento** (**Ctrl+Alt+N**) levam o teclado para lá a partir do texto; **Alt+2** volta ao editor. Desde 0.93.0, as notas (do documento e do projeto) começam como texto simples: sem fonte, tamanho, recuo da primeira linha, espaçamento de parágrafo nem estilo da formatação de texto do projeto, na fonte da interface do programa no zoom das notas. A formatação só aparece quando você mesmo a aplica (a barra de formatação, os menus, os atalhos) e então permanece. As notas que já têm texto mantêm a aparência que tinham. O texto colado nas notas é simples; **Editar > Colar com formatação** (**Ctrl+Alt+Shift+V**) mantém a formatação que ele traz.
- **Favoritos**: os favoritos do documento e os do projeto. Um clique em um favorito de um documento do projeto mostra o seu texto abaixo das listas. Você pode escrever e formatar ali como em um editor (a barra de formatação, o menu Formatar e os seus atalhos agem sobre ele enquanto o cursor está nele): tudo vai para o documento e o seu arquivo de imediato, e um editor que mostra o mesmo documento mostra a alteração de imediato. O texto ali tem um zoom próprio.
- **Mapa mental**: um quadro de notas para o documento ou a pasta (veja "Mapas mentais" abaixo).
- **Objetos**: os personagens, locais, ambientes, pontos de interesse, clima e outras coisas da sua história, cada um com uma ficha própria e vínculos entre eles (veja "Objetos" abaixo).
- **Metadados**: etiqueta, status, tipo de seção, incluir na compilação, datas de criação e modificação, **metadados personalizados** e **palavras-chave**.
- **Instantâneos** (seção 11).
- **Comentários e notas de rodapé**.

As ferramentas de escrita de complementos podem adicionar abas próprias; os botões delas vêm primeiro na linha, por exemplo **Transcrições** da Transcrição de fala (seção 10).

**O zoom próprio do inspetor.** **Ctrl+roda do mouse** sobre uma parte do inspetor torna essa parte maior ou menor, sem alterar os editores. As abas com duas partes têm um zoom para cada uma: **Sinopse e notas** (a ficha, as notas), **Favoritos** (a lista dos favoritos do documento, a lista dos favoritos do projeto, o texto do favorito abaixo delas: três zooms), **Instantâneos** (a lista, o texto do instantâneo), **Objetos** (os botões de categoria com a lista, a ficha). A lista **Zoom** na parte de baixo mostra e define o zoom da parte que tem o teclado, ou da parte em que você deu zoom por último; o seu título nomeia a parte ("Zoom: Notas"), e na aba Mapa mental ela é o zoom do quadro. As outras abas (Metadados, Comentários) e as linhas de botões no alto e embaixo compartilham um único zoom; com **Visualizar > Zoom separado nas abas do inspetor** (também no menu de clique com o botão direito da lista Zoom), Metadados e Comentários mantêm um cada um.

**O inspetor lembra**, com o projeto, qual aba estava aberta, onde você colocou os divisores dentro das abas, o favorito mostrado e o objeto mostrado na aba Objetos.

**Comentários de uma pasta.** Com uma pasta no editor (como quadro de cortiça ou estrutura de tópicos), a aba de comentários e notas de rodapé lista os comentários e as notas de rodapé de todos os documentos dentro da pasta e das suas subpastas, agrupados por documento. Um clique no nome de um documento o abre; um clique em **Comentário ▸** (ou **Nota de rodapé ▸**) abre o documento e mostra esse lugar; o texto de um comentário ou de uma nota de rodapé pode ser alterado ali mesmo. No Texto combinado (vários documentos como um só texto), a aba lista as notas de todos os documentos mostrados.

### Mapas mentais

Todo documento e pasta pode ter um **mapa mental**: um quadro em que você coloca notas em qualquer lugar, conecta-as com linhas e setas, colore-as e as agrupa em formas de fundo. Os quadros podem ser salvos como arquivos do Scapple, e os arquivos do Scapple podem ser lidos (veja *Arquivos e Scapple* abaixo). Clique no botão **Mapa mental** do inspetor (entre Favoritos e Metadados). O botão fica sublinhado quando o documento tem um mapa mental.

**Um mapa mental fixado (0.92.0).** Normalmente a aba mostra o mapa mental do documento ou pasta que você selecionou. Em vez disso, você pode fixá-lo em um item: a aba continua então mostrando o mapa mental desse item, o que quer que você selecione no Fichário ou abra no editor. Isso serve para um quadro que você quer ao seu lado enquanto escreve, como um mapa de enredo guardado em uma pasta. Três lugares fazem a mesma coisa:

- **No Fichário**, passe o mouse sobre uma linha: um pequeno alfinete aparece perto da ponta direita dela. Clique nele, e esse item fica fixado; o alfinete permanece e fica preenchido. Um clique no alfinete preenchido o libera. Clicar em um alfinete não seleciona nada.
- **Na aba Mapa mental**, o botão de alfinete à direita da pequena barra fixa o quadro que a aba mostra e o libera de novo. Enquanto um quadro está fixado, a barra nomeia o seu item (*Fixado: Enredo*, logo depois do botão de visão geral; um título longo é abreviado, e o título inteiro aparece quando você para o mouse sobre ele), para que fique claro que o quadro não pertence ao documento selecionado.
- **Nos menus**, **Navegar > Inspecionar > Fixar o mapa mental deste item** e **Navegar > Inspecionar > Liberar o mapa mental fixado** (eles não têm atalhos no início; dê alguns nas opções, se quiser).

Enquanto um item está fixado, uma barra fina na parte de baixo do Fichário diz *Mapa mental fixado:* com o seu título e tem o botão **Liberar**. No máximo um item de um projeto fica fixado; fixar outro move a fixação. Um item sem mapa mental também pode ser fixado: a aba mostra um quadro vazio para começar. Somente a aba Mapa mental é fixada; as outras abas do inspetor acompanham o documento selecionado como sempre, e um quadro aberto em uma janela grande continua sendo o que é. A fixação é mantida com o projeto neste computador quando você o fecha, e é liberada quando o item vai para a Lixeira.

**No inspetor**, o quadro preenche a aba. A pequena barra acima dele tem **Nova nota**, **Nova forma de fundo**, **Ajustar zoom**, **Menu** (todos os comandos: Arquivo, Editar, Visualizar, Notas, Formatar), o botão de **visão geral** (veja *Como olhar ao redor*) e **Abrir em janela**. A barra abaixo dele conta as notas e tem o controle deslizante de zoom. **Abrir em janela** mostra o mesmo quadro em uma janela grande, com os mesmos menus na sua barra de menus e um inspetor à direita (**Formato da nota**, **Estilos de nota**, **Configurações do documento**); as alterações em um lugar aparecem de imediato no outro. Um clique com o botão direito no quadro abre os comandos para o que está sob o mouse. Uma nova nota (botão, menu ou clique duplo) toma o teclado de imediato, também quando ele estava antes no Fichário: o que você digita substitui o texto selecionado dela (0.94.0).

Os comandos de um mapa mental estão nestes menus próprios, não nos menus da janela principal. Abaixo, **Menu > Notas > Conectar** significa: o botão **Menu** do quadro do inspetor, depois **Notas**, depois **Conectar** (na janela grande: **Notas > Conectar** da sua barra de menus). Enquanto o quadro tem o teclado, as suas teclas vêm primeiro: **Ctrl+Z** desfaz uma alteração do quadro, **Ctrl+F** busca nas suas notas, **Ctrl+P** o imprime.

**Ajuda dos mapas mentais.** **Ajuda > Ajuda dos mapas mentais** (também o botão **?** na ponta direita da barra do quadro, e **Menu > Ajuda** do quadro) mostra um guia curto com imagens: como criar notas, como desenhar uma linha (arraste uma nota sobre outra), como desenhar uma seta (mantenha **Alt** pressionada ao soltar; **Ctrl+Alt** para uma seta de volta, **Ctrl+Alt+Shift** para as duas pontas), palavras sobre uma linha e formas de fundo. O botão **Abrir quadro de exemplo** (também **Menu > Ajuda > Abrir quadro de exemplo**) abre um quadro de prática em uma janela própria, com notas para arrastar umas sobre as outras e exemplos prontos de linhas e setas. Nada no quadro de exemplo pertence ao seu projeto, e ele não é salvo.

**Notas.**

- **Clique duplo** em um lugar vazio: uma nova nota, pronta para digitar. **Ctrl+Enter** cria uma nova nota abaixo da selecionada (empilhada). Com **Ctrl** no clique duplo, a nova nota é conectada às notas selecionadas; com **Alt**, por uma seta em direção a ela; com **Ctrl+Alt**, por uma seta de volta.
- **Clique duplo** em uma nota para alterar o seu texto; **Esc** ou um clique fora termina a edição. Uma nota curta fica tão estreita quanto o seu texto; uma longa quebra na largura das novas notas (as duas coisas em **Menu > Arquivo > Opções do mapa mental…**).
- **Arraste** notas para movê-las; com **Ctrl** ao soltar, elas são copiadas. As **setas do teclado** movem as notas selecionadas quando **Menu > Editar > Setas movem notas** (**M**) está ativado (com **Shift**, de 10 em 10 px); caso contrário, elas selecionam a nota seguinte naquela direção. **Enter** ou **F2** edita a nota selecionada, **Delete** remove as notas selecionadas.
- **Arraste uma nota sobre outra** para conectar as duas com uma linha; de novo, para remover a linha. Com **Alt**, a linha recebe uma seta em direção à nota sobre a qual foi solta; com **Ctrl+Alt**, uma seta de volta; com **Ctrl+Alt+Shift**, setas nas duas pontas.
- **Clique duplo em uma linha** para colocar uma nova nota nela, ou no rótulo de uma linha para alterá-lo (**Menu > Notas > Rótulo da conexão…**).
- **Menu > Notas > Conectar** (**Ctrl+.**), **Conectar com seta** (**Ctrl+Alt+.**), **Remover setas** (**Ctrl+,**) e **Desconectar** agem sobre as notas selecionadas: a primeira que você selecionou é o centro.
- **Menu > Notas > Nova nota conectada** cria uma nota à esquerda, à direita, acima ou abaixo da selecionada, conectada a ela (com ou sem seta).
- **Menu > Notas > Dividir** (**Ctrl+Shift+K**) divide uma nota no cursor; **Mesclar** junta as notas selecionadas.
- **Menu > Notas > Empilhar** (**Ctrl+'**), **Alinhar**, **Distribuir**, **Igualar largura / altura**, **Ajustar automaticamente**, **Trazer para a frente**, **Enviar para trás**.
- **Menu > Editar > Adicionar link…** (**Ctrl+K**) vincula o texto de uma nota a uma página da web ou a um arquivo; os documentos arrastados do Fichário para o quadro tornam-se notas com links para eles. Um clique em um link o abre: um documento abre como os links de documento no texto (**Arquivo > Opções > Comportamentos**).
- **Menu > Notas > Adicionar notas ao Fichário** transforma as notas selecionadas em documentos dentro do documento ou da pasta a que o quadro pertence, na ordem de leitura do quadro.

**As formas de fundo** (**Menu > Notas > Nova forma de fundo**, **Ctrl+Shift+G**) são áreas coloridas atrás das notas. Criada com notas selecionadas, a forma as envolve. Uma forma **magnética** (**Menu > Notas > Magnética**, **Ctrl+Shift+M**) leva consigo as notas que estão sobre ela quando é movida. Uma nova forma começa cinza médio (#808080) e não magnética; **Arquivo > Opções > Extras > Mapa mental** (e **Menu > Arquivo > Opções do mapa mental…**) define o preenchimento, a borda e **Magnética** com que as novas formas começam. (Antes de 0.93.0, uma nova forma era cinza claro e magnética; uma cor que você tivesse escolhido ali permanece, e um **Magnética** guardado foi desativado uma vez.)

**O menu de clique com o botão direito de uma nota** (ou de uma forma de fundo) altera as notas selecionadas: **Cor de preenchimento**, **Cor do texto** e **Cor da borda** oferecem as cores das etiquetas do seu projeto, as seis cores da barra de formatação (vermelho, azul, amarelo, verde, branco, preto) e **Outra...** para qualquer cor, a mesma escolha em todo o {{APP}}; **Forma** é **Retângulo**, **Retângulo arredondado**, **Nuvem** ou **Irregular** (uma nota sem borda recebe uma fina, para que a forma apareça); **Borda** define a espessura; **Ícones** coloca até oito ícones em uma nota, um em cada canto e um no meio de cada lado, escolhidos entre os ícones do Fichário (**Documentos > Alterar ícone**), inclusive os seus próprios ícones. Clique com o botão direito perto do lugar de um ícone para remover apenas esse; **Remover todos os ícones** limpa todos.

**O menu de clique com o botão direito de uma linha** pertence apenas a essa linha, de modo que outras linhas das mesmas notas não são alteradas: as suas setas (**Sem setas**, uma seta para qualquer uma das notas ou **Setas nas duas pontas**), **Cor da linha**, **Ícone no meio** (um ícone desenhado no meio da linha), o rótulo, **Inserir uma nota na linha** e **Desconectar**. Cada alteração pode ser desfeita separadamente.

Os ícones, as cores de linha e os ícones de linha são acréscimos do próprio {{APP}}; eles são mantidos no arquivo do quadro em elementos que o Scapple não lê (um quadro exportado como arquivo do Scapple tem as notas, linhas e setas sem eles). **Arquivo > Opções > Extras > Mapa mental** define o tamanho dos ícones e a forma das novas notas.

**Aparência.** **Menu > Formatar** tem negrito, itálico, sublinhado, riscado, maior e menor (**Ctrl+Shift+>** e **Ctrl+Shift+<**), cores do texto, do preenchimento e da borda, espessura da borda (nenhuma, 1 a 5 px) e estilo (arredondado, quadrado, irregular, nuvem), sombra e **Esmaecer** (**Ctrl+Shift+F**, a nota é mostrada pálida). Os **Estilos de nota** guardam uma aparência sob um nome; **Menu > Formatar > Estilo de nota** aplica um (**Ctrl+Alt+1** a **9**, **Ctrl+Alt+0** para Padrão), cria um a partir da nota selecionada ou copia e cola a aparência de uma nota (**Alt+Shift+C** / **Alt+Shift+V**). As **Configurações do documento** no inspetor da janela definem o fundo, a cor do texto, a fonte e a direção da escrita do quadro (e **Definir como padrão** para novos quadros).

**Fundo.** Um clique com o botão direito em um lugar vazio do quadro (no inspetor ou na janela), **Menu > Formatar > Cor de fundo** ou o botão de fundo das **Configurações do documento** oferece as cores das etiquetas do seu projeto, as seis cores da barra de formatação, o padrão (**Padrão para temas claros** ou **Padrão para temas escuros** nos menus, **Usar padrão** nas Configurações do documento) e **Outra...**. Um quadro tem um fundo próprio para temas claros e um para temas escuros: a cor que você escolhe é mantida para o tipo de tema em uso, e o outro tipo mantém o seu. Um quadro sem cor própria mostra o padrão de **Arquivo > Opções > Extras > Mapa mental** ("Fundo dos quadros", um para temas claros e um para escuros) e o acompanha quando você o altera. **Definir como padrão** nas Configurações do documento torna o fundo do quadro o padrão do tipo de tema em uso. Um quadro mantém a sua cor de fundo também quando ainda não há nada nele.

**Zoom e lugar.** Cada quadro lembra, com o projeto, o seu zoom e a parte dele na tela, um para o inspetor e um para a janela grande. Um quadro em que você nunca deu zoom abre no zoom padrão dos mapas mentais (**Arquivo > Opções > Edição**, "Zoom padrão do texto").

**Seleção.** Clique, **Shift+clique** ou **Ctrl+clique** para adicionar, arraste uma moldura ao redor de notas, **Ctrl+A**. **Menu > Editar > Selecionar** tem notas empilhadas abaixo, notas sobre as formas selecionadas (**Alt+A**), notas conectadas (**Ctrl+Shift+A**) e grupos conectados inteiros; abaixo dele estão **Inverter seleção** (**Alt+Shift+I**) e **Desfazer seleção** (**Ctrl+D**).

**Como olhar ao redor.** Dê zoom com **Ctrl+roda do mouse**, **Ctrl++** e **Ctrl+-**, **Tamanho real** (**Ctrl+0**), **Ajustar zoom** (**Ctrl+9**), **Zoom na seleção** (**Ctrl+Shift+8**) e **Zoom aleatório** (**Ctrl+Shift+R**). Mantenha **Espaço** pressionado e arraste para mover o quadro; mantenha **Z** pressionado para ver o quadro inteiro por um instante. **Menu > Editar > Localizar** (**Ctrl+F**) localiza e substitui texto nas notas. **Menu > Visualizar > Guias de página** mostra onde as páginas terminam na impressão.

**A visão geral (0.93.0).** Um pequeno mapa no canto superior esquerdo do quadro mostra todo o quadro em miniatura, e uma moldura para a parte que você vê agora; a moldura se move e muda de tamanho conforme você rola e dá zoom. Notas, formas de fundo e linhas são desenhadas nas suas cores; o texto de uma nota só é mostrado onde ainda pode ser lido nesse tamanho, caso contrário a nota é a sua moldura colorida. Clique na visão geral para mover a visão até aquele ponto; arraste a moldura para mover a visão com ela. O zoom permanece, e um quadro que está travado (**Impedir edição**) também pode ser percorrido. A roda do mouse sobre a visão geral funciona como sobre o quadro (**Ctrl+roda** dá zoom). Arraste a alça no canto inferior direito dela para alterar a sua largura (ela começa com um quinto da largura da visão, no mínimo 80 px, e nunca é mais larga que a metade da visão); a altura acompanha a forma do que ela mostra, nunca mais que a metade da visão, e a largura diminui junto quando a altura passaria disso. A visão geral pertence apenas à visão: ela não está em exportações, impressões nem na imagem do quadro. Ela é **automática** no início: aparece quando uma parte do quadro fica fora da visão e se oculta quando tudo cabe (com uma pequena margem, para que não pisque na borda). O botão de **visão geral** na barra do quadro do inspetor (logo depois de **Menu**) alterna entre **automática** e **sempre mostrada** (o botão fica pressionado); a mesma chave é a entrada **Visão geral sempre mostrada** no menu Visualizar do quadro (no inspetor: o botão **Menu**, depois **Visualizar**; na janela grande: a sua barra de menus). A escolha e os tamanhos são mantidos neste computador, nas configurações do programa (uma escolha para todos os quadros; o tamanho da visão geral do inspetor e o da janela grande à parte); nada vai para o projeto.

**Texto, imagens e arquivos.** Cole ou solte texto para criar uma nota (um texto com linhas vazias pode virar várias notas), solte uma imagem para uma nota de imagem e solte um arquivo de texto para criar notas com os seus parágrafos.

**Arquivos e Scapple.** **Menu > Arquivo > Exportar** grava o quadro como PDF (**Ctrl+Shift+E**), imagem PNG, texto simples, lista de texto, RTF, estrutura OPML, as suas imagens ou um arquivo do Scapple (*.scap*); **Menu > Arquivo > Importar mapa mental ou arquivo do Scapple…** lê um arquivo *.scap* no quadro. **Imprimir** e **Pré-visualização de impressão** o imprimem. **Menu > Arquivo > Impedir edição** trava o quadro.

**Opções do mapa mental** (**Menu > Arquivo > Opções do mapa mental…**, e o mesmo em **Arquivo > Opções > Extras > Mapa mental** da janela principal): se **Enter** termina a edição de uma nota (então **Shift+Enter** cria uma nova linha), se **Enter** em uma nota selecionada cria uma nova, se as novas notas assumem a aparência da nota selecionada, linhas tracejadas sem setas, notas estreitas para textos curtos, quão pálidas são as notas esmaecidas e o tamanho, a largura, a cor e a borda das novas notas e formas.

**Onde o quadro é guardado.** Um quadro é salvo sozinho, como um arquivo no layout dos arquivos do Scapple (*content/<ID>/mindmap.scap* na pasta do projeto; *Files/Data/<UUID>/quillbinder-mindmap.scap* em um projeto do Scrivener, ao lado dos arquivos do próprio documento). Um documento só recebe o arquivo quando algo é colocado no seu quadro. Desfazer (**Ctrl+Z**) funciona para cada quadro separadamente.

O {{APP}} lê arquivos do Scapple como o Scapple 1.x os grava, até onde o layout deles é conhecido; não foi possível testá-los com o próprio Scapple (veja as notas da versão 0.17.0 no arquivo de status).

### Objetos: personagens, locais e mais

Os **Objetos** são as pessoas, os lugares e as coisas da sua história, cada um com uma ficha de campos: um personagem com raça, altura, cor dos olhos, traços e história de fundo; um local com o seu tipo, o seu ambiente e o seu clima; e assim por diante. Eles pertencem ao projeto inteiro, não a um documento, e são mantidos separados do Fichário. Links os conectam, de modo que você pode percorrer um pano de fundo com cliques: a ficha do personagem, a aldeia onde ela mora, a paisagem ao redor, o clima dela.

Clique no botão **Objetos** do inspetor (à direita do botão **Mapa mental**), ou escolha **Navegar > Inspecionar > Objetos do projeto: personagens, locais, ambientes, ...**. **Janela > Objetos…** (ou o botão de janela na barra da aba) mostra os mesmos objetos em uma janela grande própria, com a lista à esquerda e a ficha à direita. As alterações em um lugar aparecem de imediato no outro.

**Categorias.** Os botões coloridos no alto são as categorias: **Personagens**, **Locais**, **Ambientes**, **Pontos de interesse** e **Clima** para começar. Clique em um para ver os seus objetos. Você pode renomeá-las, dar a elas outras cores, alterar a ordem, removê-las e adicionar categorias próprias (facções, feitiços, navios ...): veja *Modelos e cores* abaixo.

**Como criar objetos.** **+** na barra da aba cria um novo objeto na categoria escolhida (na pasta do objeto selecionado); digite o nome dele no alto da ficha. O botão com a pasta cria uma **pasta** para organizar, por exemplo "Aldeias" dentro de Locais; pastas podem conter pastas. Arraste objetos e pastas na lista para movê-los. **F2** ou um clique com o botão direito renomeia, e o menu de clique com o botão direito também tem **Duplicar** e **Excluir…** (também no menu **≡** da barra). O campo **Filtro** mostra apenas os objetos cujo nome ou texto contém o que você digita.

**A ficha.** Abaixo da faixa colorida com a categoria e o nome estão os campos do modelo da categoria. Tudo o que você digita é salvo sozinho, como os seus documentos. Os tipos de campos são:

- **Texto curto** (uma linha) e **Data** (em qualquer forma que você quiser, como "primavera de 1203" ou "o terceiro inverno").
- **Número** com uma unidade (cm, anos, °C ...).
- **Texto longo**, que pode conter links (veja abaixo) e palavras em **negrito**, em *itálico*, sublinhadas, riscadas, coloridas e realçadas: com o cursor no campo, a barra de formatação, o menu Formatar e os seus atalhos (**Ctrl+B**, **Ctrl+I**, **Ctrl+U**; também na janela de Objetos) agem sobre ele. As versões 0.20 e 0.21 do {{APP}} também os mostram.
- **Escolha**: uma lista para escolher; você também pode digitar outra coisa.
- **Sim / não**.
- **Imagem**: clique na área (ou solte uma imagem sobre ela) para escolher uma; ela é copiada para o projeto e reduzida se for maior que 1600 pixels. Um clique com o botão direito a remove.
- **Link para objetos**: os objetos vinculados são mostrados como botões coloridos. **+** adiciona um: digite para buscar, depois escolha um objeto, ou escolha **Novo em Locais: "Ashvale"** para criar um novo objeto com esse nome e vinculá-lo de imediato. Um clique em um objeto vinculado mostra a ficha dele; um clique com o botão direito remove o vínculo.
- **Link para documentos**: documentos do Fichário que pertencem ao objeto, como "Aparece em" de um personagem ou "Cenas ambientadas aqui" de um local. Um clique abre o documento no editor.

**Botão de cor.** O botão **Cor** da faixa dá a um objeto uma cor própria (ou devolve a cor da categoria), por exemplo para marcar os personagens principais.

**Links dentro de textos longos.** Em um campo de texto longo, selecione uma palavra e escolha **Link para objeto…** (ou **Link para documento…**) no menu de clique com o botão direito dele; sem uma seleção, o nome do objeto é colocado como um link. Um clique no link o segue; **Remover link** no menu de clique com o botão direito o retira de novo. No texto de um documento, **Inserir > Link para objeto…** coloca no texto um link para um objeto; um clique nele mostra o objeto no inspetor. Quando o manuscrito é compilado, esses links se tornam palavras comuns.

**Ir e voltar.** **‹** e **›** na barra da aba voltam e avançam pelos objetos que você olhou, como um navegador da web.

**Referenciado por.** Na parte de baixo de cada ficha, **Referenciado por** lista os objetos que apontam para este (em um campo de link ou em um texto), de modo que a aldeia mostra quem mora nela. Um clique leva até lá.

**Neste documento.** Quando o documento no editor está vinculado a objetos (por um campo de link de documentos, por exemplo "Aparece em"), a aba mostra **Neste documento:** com esses objetos, e o botão **Objetos** fica sublinhado como as outras abas com conteúdo.

**Exclusão.** Excluir um objeto pergunta antes. Os links para ele são removidos dos outros objetos (nos textos, a palavra vinculada permanece, como texto simples). Um link para ele em um documento diz que o objeto não existe mais quando você clica nele. Excluir uma pasta mantém os seus objetos; eles sobem um nível.

**Modelos e cores.** **Projeto > Modelos de objeto e cores…** (também no menu **≡** da aba e no menu de clique com o botão direito de um botão de categoria) edita as categorias e os seus campos:

- À esquerda, as categorias: **Adicionar**, **Remover** e as setas para a ordem. Remover uma categoria que tem objetos pergunta se eles também devem ser excluídos; **Não** não altera nada.
- À direita, o nome e a **Cor** da categoria, e os seus campos: **Adicionar campo**, **Remover**, **Para cima**, **Para baixo**. Para cada campo, o seu nome e o seu tipo; um número pode ter uma unidade, uma escolha a sua lista de opções (uma por linha), e um campo de link as categorias às quais pode se vincular (nenhuma marcada: todas).
- **Usar em novos projetos** mantém essas categorias e modelos para os projetos que você criar daqui em diante. **Modelos padrão** redefine uma categoria para o modelo com que ela veio, ou adiciona as categorias padrão que você removeu.
- Remover um campo de um modelo oculta o que os objetos têm nele, mas não o exclui dos arquivos deles. Para um campo padrão, **Modelos padrão > Redefinir esta categoria para o padrão** o traz de volta com o seu conteúdo; um campo seu não volta, portanto renomeie um campo em vez de removê-lo se você pode querer o conteúdo dele de novo.

**Onde os objetos são guardados.** Na pasta do projeto, em uma pasta *objects*: um pequeno arquivo por objeto (*objects/characters/<ID>.json* e assim por diante), os modelos, cores e pastas em *objects/schema.json* e as imagens em *objects/images*. O próprio arquivo do projeto não é alterado, de modo que as versões mais antigas do {{APP}} abrem o projeto como antes e deixam os objetos em paz. Um projeto no formato do Scrivener guarda os seus objetos na sua cópia de trabalho (seção 14); o Scrivener não os conhece. Os backups os incluem.

---

## 8. Como organizar o seu projeto

### Palavras-chave

As palavras-chave são etiquetas, por exemplo os personagens ou lugares de uma cena. Adicione-as no inspetor ou gerencie todas com **Projeto > Palavras-chave do projeto** (**Ctrl+Alt+K**), onde você pode dar cores a elas, atribuí-las aos documentos selecionados e encontrar todos os documentos com uma palavra-chave. Você também pode **arrastar palavras-chave** dessa janela para documentos no Fichário, em uma coleção, no quadro de cortiça ou na estrutura de tópicos (se o documento é um de vários selecionados, todos eles recebem as palavras-chave), ou para a lista de palavras-chave do inspetor ou de um painel de Referência rápida. Mantenha **Alt** pressionada ao soltar para atribuir também o grupo de uma palavra-chave. Dentro da janela, arraste uma palavra-chave sobre outra para colocá-la nesse grupo. As cores das palavras-chave aparecem nas fichas.

### Metadados personalizados

Em **Projeto > Configurações do projeto > Metadados personalizados** você pode adicionar campos próprios: texto, caixa de seleção, lista (um conjunto fixo de opções) ou data, por exemplo *Ponto de vista*, *Local* ou *Dia da história*. Eles aparecem no inspetor e como colunas da estrutura de tópicos.

### Como salvar as configurações do projeto em um arquivo

**Projeto > Salvar configurações do projeto em arquivo…** grava as configurações do projeto aberto em um arquivo próprio (*.qbsettings*): etiquetas, status, tipos de seção, metadados personalizados, palavras-chave, a formatação padrão, os estilos, o formato de roteiro, as metas, as configurações de compilação, a visualização de página e o tamanho da página, as configurações de impressão, a aparência (tema, cores, fontes, largura da coluna, borda), as opções próprias do projeto (Arquivo > Opções com "Este projeto") e os modelos e cores da ficha de Objetos. O texto dos seus documentos não faz parte dele. Etiquetas, tipos de seção e as pastas que as configurações de compilação nomeiam são guardados pelos seus nomes, de modo que o arquivo pode ser lido em outro projeto.

**Projeto > Carregar configurações do projeto de um arquivo…** lê esse arquivo no projeto aberto. Quando as configurações do projeto são as mesmas do arquivo, ele diz isso e não altera nada. Quando elas diferem, uma janela lista cada seção com o que difere (por exemplo "somente no arquivo: Ideia; somente no projeto: Revisar") e permite que você escolha para cada seção:

- **Usar o do arquivo**: as configurações do arquivo para esta seção.
- **Manter o do projeto**: a seção permanece como está.
- **Padrão do Quillbinder**: o que um novo projeto recebe (as configurações para novos projetos de **Arquivo > Opções**, as etiquetas e os status padrão ...).

**Usar tudo do arquivo** e **Manter tudo do projeto** definem todas as seções de uma vez. Seja o que você escolher, nada que os seus documentos usam se perde: uma etiqueta, um status, um tipo de seção, uma palavra-chave, um campo de metadados personalizados, uma categoria de objetos ou um estilo que documentos ainda usam permanece no projeto mesmo quando o arquivo não o tem, e a mensagem no final nomeia o que foi mantido. Quando a formatação ou um estilo muda, o texto de todos os documentos acompanha, como quando você mesmo o altera. Antes de carregar, o {{APP}} guarda as configurações como estavam em um arquivo na pasta *settings* do projeto ("settings-before-loading-*data*.qbsettings"); carregue esse arquivo para voltar.

### Tipos de seção

Os tipos de seção dizem à Compilação o que cada documento é: *Parte*, *Capítulo*, *Cena*, *Seção* e assim por diante. Defina-os por documento no inspetor, ou deixe o {{APP}} decidir pela posição (**Configurações do projeto > Tipos de seção**, "Tipos padrão por estrutura"), por exemplo: as pastas no primeiro nível são capítulos, os documentos de texto dentro delas são cenas.

### Modelos de documento

Coloque documentos de modelo (por exemplo uma ficha de personagem) em uma pasta e escolha essa pasta em **Configurações do projeto > Pastas especiais**. Os modelos aparecem em **Projeto > Novo a partir de modelo**. Em **Documentos > Modelo padrão para subdocumentos** você pode fazer uma pasta usar um modelo para todo novo documento que você criar dentro dela.

### Modelos de projeto

**Arquivo > Salvar como modelo…** salva o projeto atual (estrutura, configurações e quaisquer documentos de exemplo) como um modelo para novos projetos.

---

## 9. Busca

### Busca rápida

Digite algumas letras no campo da barra de ferramentas. Uma lista mostra os documentos cujo **título**, **sinopse** ou **texto** as contém, até dez de cada tipo sob o seu próprio título. Escolha um para abri-lo: os lugares do texto que contêm o que você digitou ficam **marcados**, e o primeiro é selecionado e rolado para a vista. As marcas desaparecem quando você escolhe outro documento no Fichário. Este é o caminho mais rápido até um documento cujo nome você conhece.

### Busca no projeto

**Ctrl+Shift+F**, ou a lupa na barra de ferramentas, mostra um campo de busca acima do Fichário. Digite, e o Fichário é substituído pela lista roxa de resultados.

- Clique em um resultado para abri-lo. **As ocorrências são realçadas no texto**, a primeira é selecionada e rolada para a vista, e **F3** / **Shift+F3** saltam de uma para a seguinte. **Ctrl+F** abre a barra de busca já preenchida com o seu termo de busca.
- O **X** no cabeçalho dos resultados volta ao Fichário; o editor mantém o documento que mostra. Clique de novo na lupa da barra de ferramentas para ocultar o campo de busca.
- Com as coleções mostradas, a aba **Resultados da busca** executa de novo a sua última busca. A última busca é mantida com o projeto.

A lupa à esquerda do campo de busca abre as opções:

| Grupo | Entradas |
|-------|----------|
| **Buscar em** | **Tudo**, ou um entre: Título, Texto, Sinopse, Notas, Palavras-chave, Etiqueta, Status, Metadados personalizados. Um clique escolhe esse lugar. Mantenha **Alt** pressionada ao clicar (ou ao abrir o menu) para escolher vários lugares. |
| **Operador** | **Todas as palavras** (toda palavra deve ocorrer, em qualquer ordem), **Qualquer palavra**, **Frase exata**, **Expressão regular** |
| **Opções** | Palavras inteiras, Diferenciar maiúsculas e minúsculas, Inverter resultados (os documentos que *não* correspondem), Buscar somente no Rascunho, Buscar somente na seleção do Fichário (os documentos selecionados no Fichário e tudo o que há dentro deles), Buscar documentos "incluídos" / Buscar documentos "excluídos" (por **Incluir na compilação**), Incluir a Lixeira |

Exemplo: para ver tudo o que ainda tem o status "A fazer", busque por `To Do` e escolha **Status** em Buscar em.

**Salvar busca como coleção…** na parte de baixo do menu guarda a busca, com as suas opções, como uma coleção que é executada de novo a cada vez que você a abre.

### Substituir no projeto

**Editar > Localizar > Substituir no projeto** substitui um texto no projeto inteiro de uma vez.

1. Digite o texto a **Substituir** e o texto a colocar no lugar dele (**Por**). Deixe "Por" vazio para remover o texto.
2. Escolha **Diferenciar maiúsculas e minúsculas**, **Somente palavras inteiras** ou **Expressão regular** (então "Por" pode usar `\1` a `\9` para os grupos da expressão).
3. Marque onde substituir: **Títulos**, **Texto**, **Notas**, **Sinopses**, **Metadados personalizados** (campos de texto), **Comentários e notas de rodapé** (os textos mostrados no inspetor).
4. Escolha **Projeto inteiro** ou **Somente os documentos selecionados** (os documentos selecionados no Fichário, com o que há dentro deles). A Lixeira é deixada em paz, a menos que você a marque.
5. **Contar** diz quantas ocorrências mudariam e onde. **Substituir tudo** pergunta mais uma vez e então faz a substituição.

Substituir no projeto não pode ser desfeito como um todo. Deixe marcado **Fazer um instantâneo de cada texto antes de ele ser alterado**: cada texto alterado tem então um instantâneo chamado "Before replacing …" para voltar a ele (seção 11). Títulos e sinopses não têm instantâneos; para uma alteração grande, faça um backup antes com **Arquivo > Backup > Fazer backup agora**. Um texto que está aberto em um editor também pode ser desfeito ali com **Editar > Desfazer**.

A formatação permanece como está: o novo texto assume a formatação do texto que ele substitui. Um texto que não está aberto em um editor é alterado no seu arquivo, de modo que **Editar > Desfazer** não desfaz essa alteração (o instantâneo feito antes desfaz); o mesmo vale para Converter aspas, cartões de comentário e Comparar e transferir.

### Localizar por formatação

**Editar > Localizar > Localizar por formatação** (**Ctrl+F3**) abre uma pequena janela que permanece aberta enquanto você trabalha. Escolha o que procurar:

| Localizar | Encontra | Escolha extra |
|-----------|----------|---------------|
| Texto realçado | texto marcado com o marca-texto | limitar a uma cor |
| Comentários e notas de rodapé | texto com um comentário ou nota de rodapé anexado (inspetor) | limitar a uma cor de comentário |
| Anotações no texto, notas de rodapé no texto | notas digitadas no texto | cor da anotação |
| Cor de revisão | texto digitado no modo de revisão | qual revisão |
| Texto colorido | texto cuja cor você definiu manualmente | limitar a uma cor |
| Links | links da web e links para documentos | |
| Formato de caractere | texto em negrito, itálico, sublinhado, riscado (definido manualmente, não por um estilo) | qual deles |
| Estilo | texto com um estilo de parágrafo ou de caractere | qual estilo |
| Imagens | imagens no texto | |

**Contendo o texto** restringe a busca aos lugares que contêm uma palavra (para comentários e notas de rodapé, o texto da própria nota também conta). **Próximo** e **Anterior** selecionam o lugar seguinte. Com **Buscar em: Todos os documentos**, a busca continua nos documentos seguintes do Fichário e os abre; com **Somente o editor atual** ela permanece onde você está. **Ctrl+G** e **Ctrl+Shift+G** repetem a busca sem a janela. **Fechar** (ou **Esc**) fecha a janela.

---

## 10. Metas, estatísticas e ferramentas de escrita

- **Projeto > Metas do projeto** (**Ctrl+Shift+T**): defina uma meta de palavras ou de caracteres para o Rascunho e para cada sessão de escrita e, se quiser, um prazo e seus dias de escrita. O {{APP}} pode então calcular quanto você precisa escrever a cada sessão. O rodapé mostra seu progresso ao lado da contagem de palavras (desde a 0.94.0): uma barra pequena e, por exemplo, "83 / 1.000 hoje" para a sessão e "12.400 / 80.000 rascunho" para o Rascunho, cada uma só enquanto a sua meta estiver definida; um clique nela abre esta janela. As duas barras finas sob a caixa de busca rápida (azul: Rascunho; laranja: sessão) continuam lá, e **Arquivo > Opções > Aparência > Interface geral** pode desativá-las. Palavras que entram no projeto de uma só vez não contam como escritas na sessão (0.94.0): texto importado (**Arquivo > Importar**, **Importar e dividir**, arquivos soltos na janela, páginas da web), uma colagem de mais de 200 palavras e as palavras que um projeto já contém quando você o abre pela primeira vez. **Redefinir sessão agora** inicia a contagem em 0, e a dica da caixa de busca rápida mostra isso na hora. A janela das metas reserva o espaço de suas barras, de modo que os campos não se mexem quando você define uma meta.
- **Metas de documento**: clique na meta no rodapé do editor.
- **Metas de corte** (0.95.0): uma meta pode ser um número negativo, em todos os campos de meta (o Rascunho, a sessão e um documento). É uma meta para cortar: "-2.000" para uma sessão significa cortar 2.000 palavras hoje, "-4.000" para um documento significa reduzi-lo a 4.000 palavras. A barra dela enche à medida que a contagem cai (a barra de uma sessão pelas palavras removidas, a de um documento ou do Rascunho até a contagem chegar ao número), e um número negativo é mostrado em vermelho. Com uma meta de sessão negativa, a contagem de hoje é líquida: ela conta as palavras removidas como um número abaixo de 0, em vermelho; com uma meta positiva, continua como antes e nunca fica abaixo de 0. O Scrivener não aceita metas negativas: uma meta negativa é mantida apenas na cópia própria do {{APP}} de um projeto no formato Scrivener; o *.scriv* recebe 0 ("sem meta") no lugar dela.
- **Projeto > Estatísticas do projeto**: palavras e caracteres no Rascunho, no texto compilado e no projeto inteiro, além de uma estimativa do número de páginas. Em um projeto grande, a contagem leva um momento; o ponteiro de ocupado indica isso.
- **Projeto > Histórico de escrita**: quantas palavras você escreveu em cada dia.

As ferramentas de escrita abaixo são complementos (seção 17): **Gerador de nomes**, **Bloco de rascunho** e **Estatísticas do texto** (que vieram com a atualização 0.11 em um único pacote) e **Transcrição de fala** (0.61); cada um pode ser desativado ou removido em **Ajuda > Complementos e atualizações > Gerenciador de complementos**. Sem eles, **Editar > Ferramentas de escrita** avisa isso; o Gerenciador de complementos fica no menu Ajuda.

### Estatísticas do texto

**Projeto > Estatísticas do texto…** conta uma parte do seu projeto. Escolha no alto qual texto:

- **Texto selecionado** (escolhido quando você abre a janela com texto selecionado),
- **Texto no editor** (no texto combinado: todos os documentos exibidos),
- **Documentos selecionados no Fichário**, com os documentos dentro deles,
- **Rascunho (incluído na compilação)**, ou o **Projeto inteiro** (sem a Lixeira).

A janela tem quatro abas:

- **Visão geral**: palavras (contadas exatamente como na barra do rodapé e nas metas), caracteres com e sem espaços, parágrafos, frases, palavras diferentes, páginas de 250 palavras, tempo de leitura a 230 palavras por minuto e médias (letras por palavra, palavras por frase e por parágrafo, a frase mais longa; aponte para ela para lê-la).
- **Legibilidade (estimativas)**, também na primeira aba. Para o inglês: o *Flesch Reading Ease* (de 0 a 100, quanto mais alto, mais fácil; de 60 a 70 é inglês simples) e o *Flesch-Kincaid Grade Level* (o ano escolar dos EUA necessário para acompanhar o texto). Para o alemão: o *índice de Flesch de Amstad* (a mesma escala) e a *Wiener Sachtextformel* (aproximadamente o ano escolar, de 4, fácil, a 15, muito difícil). Para todos os idiomas: o *LIX* (palavras por frase mais a proporção de palavras com mais de seis letras; abaixo de 30, muito fácil; acima de 60, muito difícil). O idioma é detectado automaticamente ou escolhido no alto. Essas pontuações são orientações, não veredictos: sílabas e frases são identificadas por regras práticas, e as fórmulas foram criadas para prosa informativa, não para diálogos nem poesia.
- **Frequência de palavras**: cada palavra com sua contagem e sua participação, das mais frequentes às menos. **Ocultar palavras comuns** deixa de fora artigos, pronomes, preposições e similares (inglês e alemão). Defina um comprimento mínimo, procure uma palavra ou conte as maiúsculas separadamente. Clique duas vezes em uma palavra para buscá-la no projeto.
- **Frases repetidas**: grupos de duas a seis palavras que você usou mais de uma vez dentro de frases, como uma descrição que você deu duas vezes. Frases formadas só por palavras comuns ficam de fora. Clique duas vezes para buscar no projeto.
- **Documentos**: uma tabela dos documentos com palavras, caracteres, frases, palavras por frase e sua participação no todo. Clique no título de uma coluna para ordenar; clique duas vezes em um documento para abri-lo.

**Exportar tabela…** salva a tabela da aba atual como um arquivo CSV para uma planilha.

### Gerador de nomes

**Editar > Ferramentas de escrita > Gerador de nomes…** cria nomes para personagens.

1. Marque as listas dos **prenomes** (62 origens da Europa, das Américas, da Ásia e da África, por exemplo inglesa, americana, irlandesa e gaélica irlandesa, alemã, austríaca, suíça, holandesa, francesa, franco-canadense, italiana, espanhola, mexicana, portuguesa, brasileira, dinamarquesa, sueca, norueguesa, finlandesa, islandesa, polonesa, tcheca, húngara, russa, grega, turca, hebraica, árabe, persa, indiana, japonesa, chinesa, coreana, iorubá, suaíli e zulu; ainda não há listas escocesas nem galesas). Os **sobrenomes** vêm da mesma lista do prenome, ou desmarque **A lista do prenome** e escolha outras listas (um prenome italiano com um sobrenome irlandês).
2. Escolha **Gênero**, **Número de nomes** e o que **Mostrar** (nome completo, só o prenome, só o sobrenome, iniciais e sobrenome). **Com nome do meio** acrescenta um segundo prenome.
3. **Ajuste fino**: **comum ↔ raro** sorteia entre os nomes mais comuns (à esquerda) ou os raros (à direita); isso funciona com as listas marcadas como *por frequência* (por exemplo americana, sueca, húngara, hebraica), que trazem a frequência de cada nome. **Aliteração** dá essa proporção dos nomes um sobrenome com a primeira letra do prenome (Peter Parker); **Sobrenomes duplos** une dois sobrenomes com hífen. **Prenome começa com** / **Sobrenome começa com** limitam os nomes aos que começam com essas letras.
4. Clique em **Gerar nomes**. Selecione nomes e clique em **Copiar** (todos os nomes quando nenhum está selecionado) ou em **Inserir no texto** (também com clique duplo) para colocá-los no cursor do editor.

As mulheres recebem a forma feminina do sobrenome onde o idioma a tem (polonês *Kowalska*, tcheco *Nováková*, russo *Иванова*). Nomes japoneses, chineses, coreanos, húngaros e vietnamitas mostram o sobrenome primeiro. Algumas listas estão em sua própria escrita (russo, grego, hebraico, árabe e outras); o nome da lista indica qual.

**Adicionar lista própria…** transforma um arquivo de texto com um nome por linha em uma lista sua (prenomes, prenomes femininos ou masculinos, ou sobrenomes). Escolha a lista e clique em **Remover lista própria** para excluí-la. As listas de nomes vêm do projeto de código aberto *Faker* (licença MIT); são listas de nomes em uso, não listas de nomes históricos verificadas.

### Bloco de rascunho

O bloco de rascunho guarda anotações fora de qualquer projeto: ideias enquanto outro projeto está aberto, citações copiadas de uma página da web, a imagem de uma parte da tela. Abra-o com **Janela > Bloco de rascunho** (**Alt+Shift+Return**).

- **+** inicia uma nota (a seta ao lado: texto formatado ou simples). O título acima do texto é o nome da nota. **−** exclui a nota selecionada.
- Notas formatadas aceitam **negrito**, *itálico* e sublinhado (botões ou **Ctrl+B / I / U**) e imagens: cole uma imagem ou clique na **câmera** para capturar uma parte da tela. A câmera existe apenas para notas formatadas.
- Tudo é salvo automaticamente enquanto você digita. Se uma nota não puder ser salva (disco cheio, pasta de notas que não existe mais), a linha de status avisa e a nota continua aberta com seu texto: outra nota, uma nota nova ou outra pasta de notas só abre depois que ela for salva, e uma janela oferece **Copiar texto** e **Tentar novamente**. Se o programa terminar enquanto uma nota não pode ser salva, seu texto vai para a pasta de emergência *~/.local/share/{{APP}}/Emergency* (0.65).
- **Enviar para o projeto** lista todos os projetos abertos, inclusive os abertos em outras janelas do {{APP}}. Para cada um: **Novo documento na pasta Pesquisa**, **Novo documento em** uma pasta à sua escolha ou **Adicionar ao fim de** um documento de texto. A formatação e as imagens vão junto. A nota permanece no bloco de rascunho até você excluí-la.

As notas são arquivos comuns em uma pasta, normalmente *Documents/{{APP}} Scratch Pad* (texto simples como .txt, notas formatadas como .html com suas imagens na subpasta oculta *.images*). Escolha outra pasta, o tipo das notas novas e se o bloco de rascunho fica acima das outras janelas em **Arquivo > Opções > Geral > Bloco de rascunho**. Notas alteradas em outro lugar (outra janela do {{APP}}, uma pasta sincronizada) aparecem na hora.

**De qualquer programa.** O atalho dentro do programa funciona enquanto o {{APP}} está com o teclado. Para abrir o bloco de rascunho a partir de qualquer programa, faça a área de trabalho iniciar o {{APP}} com `--scratch-pad`: no KDE, **Configurações do sistema > Teclado > Atalhos > Adicionar novo > Comando ou script**, digite o comando mostrado em **Arquivo > Opções > Teclado** (botão **Copiar comando**) e pressione as teclas que quiser. O bloco de rascunho então abre sozinho, sem janela de projeto, e envia as notas para os projetos que estiverem abertos. Se o {{APP}} foi adicionado ao menu de aplicativos (**Ajuda > Adicionar ao menu de aplicativos**), a entrada do menu também tem a ação **Bloco de rascunho**.

**A câmera** captura a imagem com o programa de capturas de tela do KDE, o *Spectacle*, quando ele está instalado (necessário no Wayland, onde os programas não podem ler a tela por conta própria): arraste sobre a parte desejada. Sem o Spectacle, no X11, o {{APP}} mostra a tela e você arrasta sobre a parte; **Esc** cancela, **Enter** captura a tela inteira.

### Transcrição de fala

Fale em vez de digitar: o complemento **Transcrição de fala** (versão 0.61 ou posterior) escreve o que você diz. Tudo acontece neste computador; nada é enviado a lugar nenhum enquanto você fala.

**Antes da primeira gravação: escolha um modelo.** O reconhecimento de fala precisa de um modelo, que é baixado uma única vez. Abra **Arquivo > Opções > Fala** (ou a seta ao lado do botão do microfone, **Opções de transcrição de fala…**), escolha um modelo e pressione **Baixar e usar**:

- **Small**: download de 190 MB, rápido; bom para fala clara, tradução apenas aproximada.
- **Medium** (recomendado): 539 MB; acompanha a fala na maioria dos processadores dos últimos anos; bom reconhecimento e boa tradução.
- **Large v3**: cerca de 1,1 GB; o melhor, mas em um processador costuma ser mais lento que a fala, de modo que, em trechos longos, o texto fica para trás. Uma placa de vídeo ajuda.
- **Large v3 Turbo**: 574 MB; próximo do Large v3 e mais rápido, mas não traduz.

Os modelos vêm dos arquivos de modelo do whisper.cpp no Hugging Face. Em **Origem** você pode indicar outro endereço no lugar dele, ou usar **Usar um arquivo que baixei…** para um arquivo de modelo que você mesmo obteve. Antes de um modelo ser usado, ele é verificado: primeiro sua soma de verificação (SHA-256), depois a descrição do modelo dentro do arquivo e uma breve transcrição de teste. Só quando tudo está certo ele substitui o modelo em uso, que então é excluído. Um modelo da origem oficial precisa ter a soma de verificação conhecida; caso contrário, não é instalado (o download pode estar danificado). Um arquivo de outro endereço, ou um que você mesmo baixou, cuja soma de verificação não seja a conhecida só é lido depois que você concordar, porque o mecanismo de fala precisa lê-lo. Outros endereços devem começar com *https://*. Um download interrompido continua de onde parou na próxima vez. **Teste de velocidade** mostra quanto tempo este computador leva para uma gravação curta com o modelo e o dispositivo escolhidos.

**Gravação.** Pressione o botão do microfone na barra de ferramentas Ferramentas (ou **Editar > Ferramentas de escrita > Transcrição de fala**). O inspetor abre na aba **Transcrições** (o microfone, o primeiro na fileira de botões do inspetor) e uma nova transcrição começa, com o nome formado pela data e hora. Fale; uma pausa curta encerra um trecho, e o texto dele aparece alguns segundos depois. Uma pausa mais longa (3 s, a menos que você a altere) inicia um novo parágrafo. Pressione o botão de novo para parar: os trechos que ainda aguardam são escritos e, em seguida, o texto pode ser editado. A seta ao lado do botão escolhe o microfone.

**As transcrições** são arquivos de texto simples em uma pasta própria, fora de qualquer projeto (normalmente *Documents/{{APP}} Transcriptions*), de modo que seus projetos permanecem como estão. Na aba você pode editar uma transcrição (salva automaticamente), renomeá-la, copiar o texto inteiro para colá-lo onde precisar, excluí-la (ela vai para a lixeira da área de trabalho) ou abrir a pasta. Enquanto uma gravação está sendo escrita, o texto dela é somente leitura. Se uma transcrição editada não puder ser salva, ela continua aberta (a linha de status diz o motivo); quando uma gravação começa ou o programa termina nesse meio-tempo, o texto dela vai para a pasta de emergência *~/.local/share/{{APP}}/Emergency* (0.65).

**Gravar agora, escrever o texto depois** (0.64). Escrever o texto exige muito poder de processamento; em um notebook na bateria (ou em um celular), você pode apenas gravar e deixar o texto para ser escrito depois. Escolha **Apenas gravar** em **Arquivo > Opções > Fala** (*Quando o microfone inicia*; no Android é o padrão). O botão do microfone então apenas grava: nada mais é executado, nenhum modelo é carregado. A gravação é mantida como um arquivo de som (WAV, cerca de 1,9 MB por minuto) junto das transcrições, com o mesmo tipo de nome, e aparece na lista da aba com um botão **Transcrever** em sua linha; **Transcrever tudo** escreve o texto de todas as gravações que ainda não têm, uma após a outra. O texto então aparece como em uma gravação ao vivo e pode ser editado. Uma gravação é mantida depois que seu texto é escrito, a menos que você marque **Excluir a gravação depois que o texto for escrito**; renomear ou excluir uma transcrição leva a gravação junto. Para escrever o texto em outro computador, escolha uma pasta de transcrições que seu programa de sincronização mantenha igual nos dois. Se uma gravação for interrompida enquanto seu texto é escrito (ou o programa terminar), ela mantém o botão **Transcrever** e pode ser executada de novo.

**Idiomas.** O **idioma principal** é o idioma do texto; o **idioma alternativo** é outro idioma que você também fala (os dois são escolhidos entre os onze idiomas do programa: inglês, alemão, espanhol, francês, português, italiano, russo, indonésio, turco, polonês e holandês). Com o inglês como idioma principal, o que você diz no idioma alternativo é escrito em inglês (o whisper só traduz para o inglês). Palavras do idioma alternativo dentro de frases em inglês podem sair mal compreendidas (em um teste, "Bahnhof" virou "Bon Huff"); coloque as que você usa em **Palavras de dica**, e elas serão escritas como você as diz ("Bahnhof", "Brötchen"). Com outro idioma principal, ou com o Large v3 Turbo, a fala permanece no idioma em que foi dita. **Manter o que foi dito ao lado da tradução** acrescenta o original entre parênteses depois de um trecho traduzido (leva mais tempo). **Palavras de dica**: nomes e palavras que você usa, para que sejam escritos do seu jeito.

**Outras opções.** **Calcular em**: o processador, ou uma placa de vídeo que o whisper.cpp possa usar por meio do Vulkan (mostrada quando uma é encontrada; placas com memória insuficiente para o modelo aparecem na lista, mas não podem ser escolhidas). No Windows, por enquanto o complemento usa apenas o processador: Small ou Medium acompanham melhor ali. **Threads do processador**: uma a menos do que o processador tem mantém a digitação fluida. **Precisão**: *Cuidadosa* pondera vários textos possíveis e detecta primeiro o idioma de cada trecho: melhor, porém mais lenta. **Microfone**, **Sensibilidade** (baixa para um ambiente barulhento, alta para uma voz baixa), **Pausa que encerra um trecho**, **Novo parágrafo após uma pausa de** (*nunca* a desativa), a **pasta** e, em **Avançado**: omitir texto no silêncio e no ruído (o whisper às vezes inventa ali uma linha como "Thank you for watching") e usar o texto anterior como contexto para o trecho seguinte.

---

### Assistente de tradução

Desde a 0.92.0, o complemento **Assistente de tradução** (versão 1.5; até a 1.4 o nome era Tradução e revisão) é um dos complementos próprios do {{APP}} e vem dentro da versão completa; para um {{APP}} que você já tem, é um pacote à parte na página de Releases (seção 17). Ele traduz entre quaisquer dois de onze idiomas e verifica a ortografia e a gramática de textos em inglês, no seu próprio computador: o texto nunca sai dele. O próprio {{APP}} não tem funções de IA; este complemento trabalha com modelos de linguagem e pode ser desativado ou removido no Gerenciador de complementos como qualquer outro.

- **Esta ferramenta dá apenas sugestões. As traduções dela não são precisas: confira cada uma, o que exige bom conhecimento dos dois idiomas.** A mesma frase está na janela do complemento e na sua descrição.

- Ele tem uma aba própria no inspetor e uma janela própria (seu comando está no submenu Ferramentas de escrita do menu Editar, e há um botão na barra de ferramentas). O texto entra, o resultado sai com cada alteração marcada, ao vivo enquanto você digita; um clique em uma palavra mantém o original ou oferece outras palavras.
- Ele precisa de um modelo, que você escolhe e baixa uma vez nas opções do complemento (de algumas centenas de megabytes a vários gigabytes; um modelo grande demais para o computador não é oferecido). Os modelos são obtidos das releases *models-1*, *models-2*, *models-3* e *models-4* do repositório do {{APP}}; cada um só é instalado quando sua soma de verificação é a conhecida e, depois disso, o complemento funciona sem nenhuma conexão. Um modelo, o CoEdIT Large, só pode ser usado para fins não comerciais; o complemento avisa isso antes de ele ser baixado.
- **Uma palavra isolada** (1.3). Uma palavra em inglês: o resultado é a palavra como deve ser e, abaixo dela, o que significa (os significados vêm do dicionário WordNet 3.0, embutido no complemento; uma palavra que ele não tem recebe um significado escrito pelo modelo Qwen3 que você baixou, com um aviso de que pode ser impreciso e, sem esse modelo, o complemento diz o que você pode baixar), suas sugestões de ortografia e palavras de sentido parecido. Uma palavra em alemão: o complemento a procura primeiro no **dicionário alemão**, um extrato do Wiktionary que você baixa separadamente nas opções do complemento (sua licença, CC BY-SA 4.0, é mostrada antes do download): a forma básica, as classes gramaticais, os significados e as palavras em inglês de cada significado. Os cinco primeiros significados de uma classe são mostrados e o restante fica atrás de "mais"; um clique em uma palavra em inglês a torna o resultado, e "Semelhantes" passa então a seguir essa palavra e sua classe gramatical. Para o que o dicionário não diz, e para uma palavra que ele não tem, o modelo pequeno traduz a palavra, e a saída avisa que pode ser imprecisa. Do inglês para o alemão funciona do mesmo jeito para uma palavra em inglês, pelas classes gramaticais que o WordNet conhece.
- **Inverter e Traduzir de volta** (1.3). **Inverter** transforma alemão para inglês em inglês para alemão e vice-versa: o resultado vai para o texto e é traduzido no sentido oposto. **Traduzir de volta** traduz o resultado de volta para o idioma do seu texto e o mostra abaixo do resultado, até você alterar o texto; precisa do modelo do caminho de volta (OPUS-MT inglês para alemão (pequeno) para um texto em alemão) e oferece baixá-lo quando ele não existe. Enquanto você traduz, a segunda linha dos controles mostra o modelo de tradução (escolher outro altera a configuração e a tradução), e a lista de estilos fica oculta.
- **Textos curtos** (1.3). Uma única palavra e um texto de até três palavras nunca são entregues ao modelo grande de tradução (ele é melhor para frases inteiras e pouco confiável para palavras isoladas): primeiro o dicionário, depois o modelo pequeno, mesmo quando o grande está escolhido. Para um texto curto de várias palavras, "Outras traduções:" lista as melhores traduções completas do modelo, sem nomes que ele inventou.
- **Versão 1.4.** Uma única palavra, em qualquer direção, agora mostra, abaixo do resultado, as traduções de cada classe gramatical que ela tem (uma palavra alemã que é verbo em minúscula e substantivo com maiúscula recebe as duas; uma palavra que o dicionário não tem é traduzida nas duas formas), depois "Traduções semelhantes" (palavras de sentido parecido com o resultado em inglês) ou, do inglês para o alemão, "Palavras alemãs semelhantes": as palavras alemãs cujos significados nomeiam a palavra inglesa ou um de seus sinônimos (isso exige o dicionário alemão; sem ele, uma linha avisa), e então a breve descrição. Com "Automático", uma única palavra alemã que não é uma palavra inglesa é reconhecida como alemã quando o dicionário alemão está instalado. Antes de o alemão ser traduzido, as letras maiúsculas dos substantivos alemães são corrigidas na cópia que vai para o modelo (o seu texto permanece como está): uma palavra que o dicionário conhece apenas como substantivo recebe a maiúscula, uma palavra conhecida nas duas formas ("essen") mantém a forma que você digitou, exceto logo depois de um artigo ou de uma preposição com artigo ("das Essen", "im Essen"); sem o dicionário, uma nota sob a tradução avisa que os substantivos alemães precisam de maiúsculas. Alternativas que diferem entre si ou da tradução apenas nas letras maiúsculas são mostradas uma só vez. A área da palavra não mostra mais uma linha "Ortografia" para palavras do resultado (as palavras em que você clica no próprio texto a mantêm), e as regras de sufixo das palavras semelhantes não leem mais "number" como "numb". Onde algo falta (um modelo de tradução, o dicionário alemão), um aviso no alto da aba e da janela do complemento o nomeia e traz um botão que abre as opções do complemento. Um modelo que não foi usado por 15 minutos é descarregado para liberar memória (**Arquivo > Opções > Revisão > Velocidade > Descarregar modelos após**; "nunca" o mantém carregado). As dicas nos campos vazios quebram em uma largura estreita.
- **Versão 1.5: onze idiomas.** O complemento traduz entre quaisquer dois entre inglês, alemão, espanhol, francês, português (Brasil), italiano, russo, indonésio, turco, polonês e holandês: com **Traduzir** escolhido, duas listas suspensas nomeiam o idioma do seu texto e o idioma da tradução. Cada idioma tem dois pequenos modelos OPUS-MT, de e para o inglês (as licenças deles, Apache-2.0 ou CC BY 4.0, são mostradas nas opções antes de um download); um par sem inglês (do espanhol para o russo, por exemplo) passa pelo inglês com os dois modelos, e uma nota sob o resultado avisa isso, porque um erro do primeiro passo é levado ao segundo. Cada idioma, exceto o inglês, também tem um **dicionário** opcional do Wiktionary (CC BY-SA 4.0, um download separado cuja licença é mostrada antes), usado para uma única palavra desse idioma como o dicionário alemão era usado. Um idioma sem dicionário utilizável recebe apenas as traduções do modelo. Uma única palavra continua sendo corrigida como sugestão: a linha de ortografia e as palavras abaixo dela dizem que são sugestões a conferir. Os textos do próprio complemento vêm nos onze idiomas do programa (o idioma que você escolheu para o {{APP}}; inglês onde um texto não está traduzido). As configurações da 1.4 continuam funcionando: o dicionário alemão e os modelos que você baixou permanecem, e do alemão para o inglês é a primeira escolha. As opções dele agora ficam em **Arquivo > Opções > Assistente de tradução**.
- Modelos que você baixou com a versão 1.1 permanecem como estão. A versão 1.1 obtinha os modelos do antigo repositório do complemento; a 1.2 conhece o novo lugar.
- A descrição do próprio complemento, com suas regras de revisão (ortografia americana, aspas curvas, sem travessões) e no que cada modelo é bom, está em *addons/translation-proofreading/source/README.md* no código-fonte do {{APP}}.

## 11. Instantâneos

Um instantâneo é uma cópia salva de um documento em determinado momento. Faça um antes de uma grande reescrita:

- Um documento com pelo menos um instantâneo mostra um canto dobrado (claro e cinza) no alto à direita de seu ícone no Fichário.
- **Ctrl+5** faz um instantâneo; **Ctrl+Shift+5** pede um título.
- Com **Arquivo > Opções > Geral > Salvando**, **Arquivo > Salvar** (**Ctrl+S**) faz um instantâneo de cada texto que você alterou desde o último Arquivo > Salvar (com o título "Salvo"); **Arquivo > Opções > Comportamentos > Instantâneos** mostra uma breve mensagem na barra de status quando instantâneos são feitos.
- Na aba de instantâneos do inspetor você pode **comparar** um instantâneo com o texto atual (acréscimos aparecem em cor, exclusões riscadas; escolha se compara palavras, frases, parágrafos ou caracteres) e **voltar** a ele. Antes de voltar, o {{APP}} pode fazer um instantâneo do texto atual, para que nada se perca. O instantâneo é lido primeiro (0.65): um que não pode ser lido por inteiro deixa o texto atual como está, um que não contém texto o substitui só depois de uma segunda pergunta e, quando o instantâneo do texto atual foi pedido mas não pode ser feito, nada é revertido. Voltar a um instantâneo também é a forma de reparar um documento cujo arquivo está danificado ou em outra codificação de caracteres (ele é mostrado somente leitura): o arquivo danificado é guardado antes como uma cópia em *settings/conflicts*, e o inspetor diz onde (0.81.1).
- **Projeto > Excluir instantâneos antigos…** (desde a 0.61.1, abaixo de Esvaziar a Lixeira) exclui todos os instantâneos do projeto, inclusive os de documentos na Lixeira, para tornar o projeto menor de novo. Os textos permanecem como estão agora; só as versões anteriores deles vão embora. Uma pergunta antes informa quantos instantâneos há, de quantos documentos e quanto espaço ocupam; isso não pode ser desfeito, portanto faça um backup antes (**Arquivo > Backup > Fazer backup agora**). O comando fica desativado enquanto o projeto não tem instantâneos. Em um projeto no formato Scrivener, os instantâneos também saem do *.scriv* (com o salvamento que se segue na hora); desde a 0.61.1 isso vale também para um único instantâneo que você exclui no inspetor.

---

## 12. Modo de composição

**F11** preenche a tela com nada além do seu texto sobre uma folha de papel. Mova o mouse até a borda inferior para mostrar a barra de controle: zoom, largura do papel, esmaecimento do fundo, rolagem de máquina de escrever, contagem de palavras e um pequeno inspetor com sinopse e notas. **Alt+Shift+Up / Down** vai para o documento anterior ou o seguinte, **Ctrl+[** e **Ctrl+]** voltam e avançam pelos documentos que você já viu. Os atalhos dos menus Formatar e Inserir (negrito, estilos, comentários ...) funcionam aqui também. **Esc** ou **F11** volta à janela principal.

**Arquivo > Opções > Comportamentos > Modo de composição** escolhe a tela em que ele abre (a atual, a principal ou uma pelo nome), se a janela principal sai do caminho (minimizada enquanto o modo de composição está aberto) e se **Esc** o fecha (**F11** sempre fecha).

A barra de controle é escura em todos os temas, de modo que seus botões, a lista de zoom e seus textos permanecem legíveis também em temas claros (0.94.0). As cores do texto, do papel e do fundo vêm do tema. Para alterá-las, por exemplo para preto sobre branco ou verde sobre preto, vá a **Arquivo > Opções > Aparência > Modo de composição**; a alteração aparece na hora, mesmo com o modo de composição aberto. Uma imagem de fundo pode ser definida por projeto em **Configurações do projeto > Pastas especiais**.

---

## 13. Compilação: como criar o manuscrito

**Arquivo > Compilar…** (**Ctrl+Shift+E**) reúne tudo que está no Rascunho em um único arquivo.

### A janela de compilação

A janela de compilação tem **Compilar para** no alto, os formatos à esquerda, os layouts de seção no meio e o conteúdo, os metadados, as opções e as substituições à direita (quatro abas com ícones; deixe o mouse sobre uma aba para ver seu nome). A coluna Formatos tem pelo menos a largura de seu nome mais longo; arraste as linhas entre as três colunas para alargar uma, e a janela guarda as larguras para a próxima vez.

1. **Compilar para**: o formato de saída. Embutidos, sem precisar de outro programa: **Documento do Word (.docx)**, **PDF (.pdf)**, **E-book (.epub)** e, para roteiros, **Fountain** e **Final Draft**. PDF e OpenDocument pelo LibreOffice, EPUB pelo Pandoc e Markdown também aparecem se o plugin e o programa correspondentes estiverem instalados (veja abaixo). Ao lado: quantos documentos e cerca de quantas palavras serão compilados.
2. **Formatos** (à esquerda), em **Meus formatos** e **Formatos embutidos**: o formato de compilação decide a aparência do manuscrito: fontes, espaçamento entre linhas, tamanho da página, margens, cabeçalhos e rodapés, a aparência de capítulos e cenas, como saem estilos, notas e citações. Os formatos embutidos (*Manuscript (Times)*, *Manuscript (Courier)*, *Modern*, *Paperback (6 x 9 in)*, *E-book*, *Default (Editor Formatting)* e *Outline (Synopses)*) não podem ser alterados. Os botões abaixo da lista: **+** (duplicar o formato selecionado para criar sua própria versão e editá-la), o lápis (editar seu próprio formato), **-** (excluí-lo), **Importar...** e **Exportar...** (um arquivo *.qbformat* para usar em outro computador ou para presentear; um formato importado cujo nome já existe recebe um número).
3. **Layouts de seção** (no meio): **Fonte** define uma fonte para o manuscrito inteiro, ou deixa isso por conta dos layouts ("Determinada pelo layout de seção"). A tabela lista cada tipo de seção do seu projeto com o layout que ele recebe neste formato; escolha outro na lista. Abaixo da tabela, uma pequena imagem mostra como começa uma seção com o layout selecionado. **Atribuir layouts de seção...** abre uma janela maior, com os tipos de seção à esquerda e imagens de todos os layouts do formato à direita. Os formatos embutidos conhecem os nomes usuais dos tipos de seção: no *Manuscript (Courier)*, por exemplo, *Parte* recebe **Título da parte**, *Capítulo* recebe **Título do capítulo** e *Cena* ou *Seção* recebe **Texto da seção**.
4. **Conteúdo** (primeira aba): o que é compilado, com o tipo de seção de cada documento e seu layout. Escolha uma pasta para compilar só parte do Rascunho (por exemplo um capítulo), acrescente elementos pré-textuais (folha de rosto) e pós-textuais (agradecimentos). **O nome do autor** (0.94.0): a folha de rosto dos modelos de projeto diz "por" e o nome do autor. A primeira compilação que escreve uma folha de rosto pergunta esse nome (nome e sobrenome; ele é guardado com as Informações do autor em **Arquivo > Opções > Geral**, de modo que é perguntado uma só vez); se você o deixar vazio, a linha "por" é omitida da folha de rosto, em todos os formatos. O campo Autor da aba Metadados da janela de compilação, se tiver um nome, é usado no arquivo compilado. Desmarque um documento para deixá-lo de fora; isso desativa sua opção **Incluir na compilação**. **Adicionar fichas de objetos** coloca uma ficha para cada objeto da aba Objetos (seção 7) depois dos elementos pós-textuais: um título por categoria (Personagens, Locais ...), depois cada objeto com seus campos; o botão ao lado escolhe as categorias. Inicialmente está desativado.
5. **Metadados** (segunda aba): título, subtítulo, autor, idioma (por exemplo *pt-BR*), assunto, palavras-chave, editora, data, descrição, a capa de um e-book (uma imagem do Fichário) e o identificador do livro. Campos vazios recebem o título do projeto, seu nome e a data de hoje. Os detalhes vão para as propriedades do documento do PDF, o e-book e o arquivo do Word; espaços reservados como `<$projecttitle>` funcionam aqui também.
6. **Opções** (terceira aba): se o texto das pastas é compilado.
7. **Substituições** (quarta aba): substituições deste projeto, feitas depois das do formato, por exemplo um nome escrito de um jeito nos seus rascunhos e de outro no livro. Cada linha pode ser ativada e desativada, e pode ser uma expressão regular, diferenciar maiúsculas de minúsculas ou valer só para palavras inteiras.
8. **Pré-visualização** mostra as páginas do manuscrito como um PDF em uma janela própria, com o formato e o conteúdo que você escolheu, antes de qualquer coisa ser salva.

A fonte, as substituições e as fichas de objetos são guardadas com o projeto como as demais escolhas (versões mais antigas do {{APP}} as ignoram).

Clique em **Compilar…**, escolha onde salvar, e o arquivo pronto abre no seu processador de texto ou leitor, se você quiser. **Salvar configurações** guarda as escolhas sem compilar.

**Fontes e formatação de parágrafo definidas manualmente.** Com o formato *Default (Editor Formatting)*, o arquivo compilado mantém tudo: fontes, tamanhos, cores, maiúsculas, espaçamento entre letras, recuos, espaçamento, espaçamento entre linhas e tabulações. Todos os outros formatos têm uma aparência própria: ela substitui fonte, tamanho, recuos e espaçamento, e mantém a cor do texto, as maiúsculas, o espaçamento entre letras, o alinhamento e as tabulações.

### Editando um formato de compilação

**Duplicar e editar…** e **Editar…** abrem o editor de formatos. A lista à esquerda contém suas páginas:

- **Texto**: fonte, tamanho, espaçamento entre linhas, recuo da primeira linha, espaço depois dos parágrafos, texto justificado, a fonte e o tamanho dos títulos, títulos em negrito. **Usar a formatação do editor do projeto** mantém a aparência do editor.
- **Layouts de seção**: os layouts do formato (adicionar, duplicar, remover) com título, prefixo e sufixo (`Chapter <$W:chapter>`), estilo e alinhamento do título, maiúsculas, linhas em branco antes do título, do texto, da sinopse e das notas, uma nova página, o separador antes da seção, sem recuo no primeiro parágrafo. A pré-visualização mostra como começa uma seção com esse layout.
- **Atribuições**: qual layout cada tipo de seção recebe neste formato.
- **Estilos**: como um estilo do seu projeto sai neste formato. **Adicionar um estilo…**, depois escolha: **Manter sua aparência**, **Alterar sua aparência** (fonte, tamanho, negrito, itálico, sublinhado, versaletes ou maiúsculas, cor, alinhamento, recuos, espaço antes e depois, espaçamento entre linhas, texto antes e depois dele, maiúsculas, começar em uma nova página; uma caixa meio marcada, ou "inalterado", deixa o estilo como está), **Transformar em texto normal** ou **Deixar o texto de fora** (por exemplo, notas para você mesmo em um estilo próprio).
- **Página**: tamanho do papel e margens, margens espelhadas para livros (interna e externa), cabeçalhos e rodapés (esquerda, centro, direita; espaços reservados como `<$p>` para o número da página e `<$title>`), páginas opostas (textos da esquerda em páginas da esquerda, textos da direita em páginas da direita), cabeçalho e rodapé na primeira página e nas páginas em que um capítulo começa, capítulos em página da direita, o número da primeira página e seu estilo (1, 2, 3 / i, ii, iii / I, II, III), o tamanho dos cabeçalhos e rodapés.
- **Notas de rodapé e comentários**: notas de rodapé no pé da página (em e-books: notas que aparecem em uma janelinha), no fim de cada capítulo ou no fim do texto (PDF e e-books; um arquivo do Word sempre tem suas notas de rodapé no pé da página), com o título das notas e seu tamanho; anotações no texto como comentários ou removidas; comentários vinculados como comentários do arquivo do Word; realces removidos.
- **Substituições**: texto substituído somente no arquivo compilado (seu projeto permanece como está): ativado ou desativado, diferenciação de maiúsculas, palavras inteiras, expressões regulares (`\1` a `\9` em "Por").
- **Transformações**: aspas retas ou tipográficas, travessões como hifens, itálico como sublinhado (a convenção de manuscritos), vários espaços como um, sem espaços no fim dos parágrafos, texto simples sem negrito, itálico, fontes nem cores.
- **Sumário, PDF e e-book**: um sumário (seu título e quantos níveis de título), marcadores de PDF para os títulos, links da web clicáveis no PDF, um arquivo por seção que começa em uma nova página em um e-book e o sumário também como uma página do e-book.

Seus formatos são guardados em *~/.local/share/{{APP}}/CompileFormats* e ficam disponíveis em todos os projetos.

### PDF

A saída em PDF embutida diagrama as páginas por conta própria: notas de rodapé no pé da página onde são referenciadas (uma longa continua na página seguinte), cabeçalhos e rodapés, capítulos em páginas da direita se o formato assim determinar, títulos mantidos junto do texto que os segue, sem linhas isoladas no alto ou no pé de uma página, um sumário com números de página clicáveis, marcadores para os títulos no painel lateral do leitor e links clicáveis. As fontes são incorporadas. As imagens mantêm o tamanho que têm no texto; uma imagem maior que a área de texto é reduzida para caber. Imagens e tabelas ao lado do texto têm o texto fluindo ao redor delas, como no editor. Uma tabela continua por quantas páginas precisar, linha por linha, com suas linhas de cabeçalho no alto de cada página.

### E-books

A saída de e-book embutida grava EPUB 3 (também com um sumário para leitores mais antigos): um arquivo por capítulo, os títulos no sumário do leitor, notas de rodapé como notas que aparecem em uma janelinha (ou no fim de cada capítulo ou do livro), imagens e a capa de **Metadados…**. As tabelas mantêm seu design; uma imagem com legenda é uma figura (ao lado do texto, se estiver assim no editor), e sua descrição é o texto alternativo da imagem, que os leitores de tela leem. O livro recebe um identificador na primeira vez em que é compilado; ele permanece com o projeto, de modo que um leitor toma uma nova versão pelo mesmo livro. Os leitores escolhem suas próprias fontes e tamanho de página, portanto as configurações de página, os cabeçalhos e os rodapés não se aplicam.

### Word

O arquivo do Word usa estilos nomeados (Título, Subtítulo, Título 1 a 3, Citação, Centralizado, Legenda e seus próprios estilos de parágrafo e de caractere), de modo que seus títulos aparecem no painel de navegação do Word. Com **Incluir um sumário**, um campo de sumário é colocado no início, e o arquivo pede ao processador de texto que atualize seus campos quando for aberto, o que preenche os números de página; se um processador de texto não fizer isso, atualize o campo você mesmo ali. Cabeçalhos de páginas opostas, a primeira página e o número da primeira página vêm das configurações de **Página** do formato. As tabelas são tabelas do Word com suas linhas, cores, células mescladas e linhas de cabeçalho (repetidas em todas as páginas no Word). Uma imagem ao lado do texto vai para uma tabela de uma célula sem linhas, junto com sua legenda, para que o Word e o LibreOffice deixem o texto fluir ao redor de ambas; a descrição de uma imagem é seu texto alternativo.

### Títulos e espaços reservados

Em um layout de seção, o título pode conter espaços reservados que a Compilação substitui:

| Espaço reservado | Resultado |
|-------------|--------|
| `<$n>` | 1, 2, 3 … |
| `<$w>` / `<$W>` / `<$WW>` | um, dois … / Um, Dois … / UM, DOIS … |
| `<$R>` / `<$r>` | I, II, III … / i, ii, iii … |
| `<$L>` / `<$l>` | A, B, C … / a, b, c … |
| `<$n:name>` | um contador separado chamado *name* (funciona com todos os estilos de número acima) |
| `<$title>` | o título do documento |
| `<$projecttitle>` / `<$shorttitle>` | o título do projeto / suas três primeiras palavras em maiúsculas |
| `<$author>`, `<$forename>`, `<$surname>` | seu nome em **Arquivo > Opções** ou nas configurações do projeto |
| `<$wc>` / `<$wc100>` / `<$wc1000>` | a contagem de palavras, exata ou arredondada |
| `<$date>` / `<$year>` | a data de hoje (aaaa-mm-dd) / o ano |
| `<$p>` | o número da página (somente cabeçalhos e rodapés) |
| `<$pages>` | o número de páginas (cabeçalhos e rodapés de arquivos PDF e impressões) |

Exemplo: o título `Chapter <$W:chapter>` se torna *Capítulo Um*, *Capítulo Dois* e assim por diante.

Espaços reservados também podem ser digitados no próprio texto (**Inserir > Numeração automática** e **Inserir > Espaços reservados**). **Ajuda > Lista de todos os espaços reservados…** lista todos eles; selecione linhas e pressione **Ctrl+C** para copiá-las.

### Estilos na compilação

Os títulos, Título, Subtítulo, Citação em bloco, Centralizado e Legenda recebem sua aparência do formato de compilação. Todos os outros estilos trazem a própria formatação (recuos, espaçamento, alinhamento, fonte, cor), em todos os formatos de compilação. Com **Default (Editor Formatting)**, as alterações que você fez nos estilos embutidos também são levadas adiante. A página **Estilos** de um formato altera isso para cada estilo (veja *Editando um formato de compilação*).

### Notas na compilação

Na página **Notas de rodapé e comentários** de um formato de compilação você escolhe onde ficam as notas de rodapé em um PDF ou em um e-book (pé da página, fim de cada capítulo, fim do texto), se as anotações no texto viram comentários no arquivo do Word ou são removidas, se os comentários vinculados são mantidos e se os realces são removidos. Em um arquivo do Word, as notas de rodapé são sempre notas de rodapé de verdade no pé da página.

### Mais formatos de saída (plugins)

O {{APP}} também traz estes formatos. Cada um precisa de um programa instalado separadamente; PDF e EPUB por meio do LibreOffice e do Pandoc são uma alternativa aos embutidos (eles partem do arquivo do Word e se parecem com ele):

| Formato | Precisa de |
|--------|-------|
| PDF (via LibreOffice) | LibreOffice (instalado na maioria dos sistemas Ubuntu e Kubuntu) |
| OpenDocument Text (.odt, via LibreOffice) | LibreOffice |
| E-book (.epub, via Pandoc) | Pandoc (instale "pandoc" no Discover) |
| Markdown (.md) | nada |

**Arquivo > Opções > Plugins** mostra quais formatos estão prontos e quais precisam de um programa que falta. Mais formatos podem ser instalados como complementos (seção 17).

### Compilando roteiros

Duas saídas são feitas para roteiros e não precisam de outro programa:

- **Fountain Screenplay (.fountain)**: um arquivo de texto simples que o Highland, o Fade In, o Beat, o Trelby e outros programas de roteiro leem.
- **Final Draft (.fdx)**: o formato de arquivo do Final Draft, que a maioria dos programas de roteiro também consegue abrir.

Nas duas, cada parágrafo mantém o que é (cabeçalho de cena, ação, personagem, parentético, diálogo, transição, plano); o outro programa diagrama o roteiro à sua maneira. Os títulos de pastas e documentos que o formato de compilação acrescenta viram seções (Fountain) ou títulos de ato (Final Draft); o texto que não é elemento de roteiro vira ação. O título do projeto e seu nome (Arquivo > Opções) formam a folha de rosto. Anotações no texto viram notas em um arquivo Fountain e são omitidas de um arquivo Final Draft.

Para Word ou PDF, escolha o formato de compilação **Default (Editor Formatting)**: ele mantém os recuos, as maiúsculas e a fonte dos elementos como você os vê no editor. Os outros formatos de compilação substituem a fonte e o tamanho, mas mantêm os recuos.

---

## 14. Importação e exportação

### Importando arquivos

**Arquivo > Importar > Arquivos…** (**Ctrl+Shift+I**) adiciona ao Fichário arquivos de texto (.txt, .md, .rtf, .docx, .odt; arquivos antigos do Word .doc com o complemento abaixo), roteiros (.fountain, .fdx), imagens, PDFs, áudio e vídeo e outros arquivos. Você também pode arrastar arquivos do gerenciador de arquivos para o Fichário. Arquivos de texto viram documentos editáveis; imagens e PDFs são exibidos no editor; áudio e vídeo tocam ali com o complemento Media Playback (seção 6). Um arquivo que não pode ser copiado para o projeto (disco cheio, arquivo que não pode ser lido) é citado em uma mensagem e não é adicionado, de modo que nenhum item vazio fica para trás (0.92.0).

A forma como os arquivos são lidos é definida em **Arquivo > Opções > Compartilhamento > Importar**: se os comentários de arquivos do Word, do LibreOffice e RTF viram anotações no texto (caso contrário, continuam como comentários ao lado do texto), se as notas de rodapé viram notas de rodapé no texto (o padrão) ou notas de rodapé vinculadas, se os estilos de parágrafo e de caractere dos arquivos são deixados de fora (o texto recebe a aparência do seu projeto; negrito, itálico, notas e links permanecem), a codificação dos arquivos de texto simples (automática: UTF-8, senão Windows Ocidental) e se arquivos *.html* viram documentos de texto em vez de documentos de página da web.

Para onde os arquivos vão: selecione uma pasta no Fichário para importar para dentro dela (os arquivos ficam por último) ou um documento para colocá-los logo abaixo dele. O Rascunho aceita apenas texto; outros arquivos soltos ali vão para a Pesquisa.

**Importar e dividir…** lê um arquivo longo e o corta em muitos documentos, por exemplo em cada linha que contém apenas `#`, ou em cada linha em branco dupla. Desde a 0.94.0 ele também pode cortar um arquivo .docx, .odt ou .md **em seus títulos**: o Título 1 vira uma pasta, o Título 2 uma pasta ou um documento (você escolhe na janela), o texto entre os títulos vira documentos e os títulos de nível mais baixo permanecem no texto. A janela mostra, como uma árvore, o Fichário que resultaria antes de você importar. O título de um documento que nenhum título nomeia segue uma regra, escrita na janela: a primeira linha de seu texto que não está vazia (encurtada para 120 caracteres quando é mais longa) ou "<nome do arquivo> <número>" quando **Usar a primeira linha de cada seção como título** está desativado.

### Arquivos antigos do Word (.doc)

O Word 97 a 2003 salvava documentos como *.doc* (modelos *.dot*). Eles viram documentos de texto com o complemento **Word 97–2003 Import** (seção 17); sem ele, um arquivo .doc é adicionado ao Fichário como está, e você é informado de qual complemento o lê. O complemento não precisa de outro programa. Ele mantém:

- o texto, inclusive grego, cirílico, escritas asiáticas e similares;
- títulos (Título 1 a 6), título, subtítulo, citações e legendas, que recebem os estilos do seu projeto; um estilo de parágrafo do documento que seu projeto também tem (mesmo nome) recebe esse estilo;
- negrito, itálico, sublinhado, riscado, versaletes e maiúsculas, sobrescrito e subscrito, realce; parágrafos centralizados, alinhados à direita e justificados;
- listas com marcadores e numeradas com seus níveis (os números em si são feitos pelo {{APP}}, portanto a numeração "Capítulo 1" de títulos não é aproveitada);
- tabelas, como uma linha por fileira, com as células separadas por tabulações;
- notas de rodapé e notas de fim (como notas de rodapé), comentários com as palavras comentadas (o nome do autor vem primeiro no comentário);
- links, o resultado de campos (datas, números de página como foram mostrados pela última vez), quebras de página e de seção, quebras de linha;
- imagens no texto (JPEG, PNG, BMP, TIFF);
- em projetos RTF, também fontes, tamanhos, cores e recuos definidos manualmente (projetos Markdown mantêm a aparência do projeto, como em toda importação).

Ficam de fora: cabeçalhos e rodapés, imagens e desenhos flutuantes, imagens nos formatos antigos de metarquivo (WMF, EMF), objetos incorporados, texto oculto e texto excluído com alterações rastreadas (o texto inserido permanece). O texto de caixas de texto é adicionado no fim do documento. Depois da importação, uma mensagem nomeia o que ficou de fora.

**Arquivo > Opções > Compartilhamento > Conversão** pode tornar o LibreOffice a primeira escolha para arquivos *.doc* (ele também lê cabeçalhos, desenhos e arquivos do Word 6/95); quando o caminho escolhido falha, o outro é tentado. Arquivos do Word 6.0 e do Word 95, e arquivos que o complemento não consegue ler, são convertidos com o LibreOffice quando ele está instalado (no Ubuntu: `sudo apt install libreoffice-writer`); caso contrário, a mensagem avisa. Arquivos protegidos por senha precisam ser abertos e salvos antes sem a senha. Um arquivo *.doc* que na verdade é um arquivo RTF, um arquivo do Word 2007 (.docx) ou uma página da web, como alguns programas os salvavam, é lido como o que realmente é, também sem o complemento.

### Importando páginas da web

**Arquivo > Importar > Página da web…** traz uma página da web para o seu projeto. O complemento **Web Page Import** (seção 17) faz o trabalho; sem ele, o comando avisa.

1. Selecione a pasta que deve receber a página (caso contrário, ela vai para a **Pesquisa**).
2. Escolha **Arquivo > Importar > Página da web…**. Se a área de transferência contém um endereço da web, ele já vem preenchido; caso contrário, digite ou cole um.
3. Clique em **Carregar** (ou pressione Enter). A página e suas imagens são obtidas, e a janela mostra o que será importado, com o número de palavras e de imagens, o autor, o site e a data.
4. Corrija o **Título** se quiser, confira as opções abaixo e clique em **Importar**.

**Somente o artigo** mantém o texto que você procurava e deixa de fora menus, anúncios, avisos de cookies, botões de compartilhamento, seções de comentários e listas de outros artigos. O {{APP}} encontra o artigo do jeito que as ferramentas de leitura fazem (vence o texto com mais frases e vírgulas e o menor número de links; nomes como "article" ou "sidebar" na página contam a favor ou contra uma parte). Se algo estiver faltando, desmarque a opção para pegar a página inteira. **Imagens** obtém as imagens do artigo (as muito grandes são reduzidas a 1600 pixels de largura). Para uma página da internet, as imagens só são obtidas de endereços públicos: uma que esteja no seu próprio computador ou em uma rede privada (mesmo por meio de um redirecionamento) fica de fora, e o resultado informa quantas. Uma página que ela mesma vem do seu computador ou da sua rede é obtida com todas as suas imagens.

**Importar como**:

- **Documento de texto** (o padrão): títulos, parágrafos, citações, listas, tabelas (uma linha por fileira), código de programa, legendas, negrito, itálico e links viram um documento comum que você pode editar, anotar, dividir e compilar.
- **Documento de página da web**: uma cópia somente leitura do artigo com suas imagens, para consulta. Ele não é contado nem compilado.

**Vendo um documento de página da web** (0.81.3). O editor mostra a página como texto formatado simples, com seu endereço e dois botões acima dele. **Abrir página salva no navegador** mostra a página como foi salva, no seu navegador da web: o {{APP}} coloca uma cópia dela na sua pasta Downloads (*Quillbinder web pages/<título>*), porque um navegador instalado como pacote em área restrita (o Firefox no Kubuntu, por exemplo) não consegue ler as pastas do próprio {{APP}}; a linha abaixo dos botões diz onde está a cópia, e o clique seguinte a substitui. No Windows, a cópia é feita em uma pasta temporária e removida quando o {{APP}} fecha, de modo que nada seja escrito fora da pasta do programa. **Abrir endereço original** abre o endereço da página, para que você a veja como ela está hoje; o botão fica desativado quando a página não tem endereço. Páginas da web que o Scrivener salvou (um arquivo *.mht* cada uma) são mostradas do mesmo jeito; um navegador como o Firefox não consegue abrir arquivos *.mht* por conta própria, então a cópia é HTML simples com suas imagens ao lado.

Seja como for, o endereço da página vira um **favorito** do documento (inspetor, Favoritos), a breve descrição da página vira sua **sinopse**, e as **notas** registram o endereço, o site, o autor, a data de publicação e quando você a obteve.

**Arrastar um link** de um navegador da web para uma pasta no Fichário, para o quadro de cortiça ou para a estrutura de tópicos importa a página diretamente, com as opções que você escolheu da última vez na janela.

Bom saber: a página é obtida como um navegador a obteria, mas sem executar seus scripts. Páginas que montam o texto com um script muitas vezes ainda o trazem em seus dados, que então são usados (sem formatação nem imagens; uma mensagem avisa). Páginas protegidas por login ou por assinatura entregam o que mostram a um visitante que não está conectado. Páginas seguras (https) precisam do OpenSSL 3 no seu sistema (parte de todo Ubuntu). Um endereço da web que leva a um arquivo PDF não é importado desta forma: baixe o arquivo e use **Arquivo > Importar > Arquivos…**.

**Roteiros.** Arquivos Fountain (*.fountain*) e arquivos Final Draft (*.fdx*) são importados como roteiros: os parágrafos recebem os elementos do formato de roteiro do seu projeto, o documento fica no modo de roteiro e a folha de rosto do arquivo vai para as notas do documento. Texto em negrito, itálico e sublinhado, notas e quebras de página forçadas são mantidos; números de cena, marcas de revisão e o aspecto de duas colunas do diálogo duplo não (as duas falas se seguem uma à outra). **Importar e dividir…** com **Cabeçalhos de cena** corta um roteiro em um documento por cena, com o nome do respectivo cabeçalho.

### Importando um projeto do Scrivener

**Arquivo > Importar > Projeto do Scrivener…** lê um projeto do Scrivener 3 ou 2 (a pasta *.scriv*; escolha o arquivo *.scrivx* dentro dela) e cria a partir dele um novo projeto do {{APP}}. O projeto do Scrivener só é lido, nunca alterado. Desde a 0.92.0, o relatório após a importação também nomeia cada arquivo que não pôde ser copiado (uma imagem, um PDF, um arquivo de áudio, a imagem de uma ficha, um mapa mental), e tal arquivo não é contado como importado; antes, ele era deixado de fora sem uma palavra.

A importação traz: a estrutura do Fichário, textos, sinopses, notas, etiquetas, status, palavras-chave, metadados personalizados, tipos de seção, marcas de incluir na compilação, instantâneos, comentários e notas de rodapé, anotações no texto, arquivos de pesquisa (imagens, PDFs e outros), coleções, favoritos do projeto e metas. As configurações de compilação não são importadas; configure-as uma vez na janela de compilação. Escolha se o novo projeto guarda seu texto como Markdown ou RTF.

Títulos e estilos: o Scrivener marca o texto com estilo dentro de seus arquivos com códigos como `<$Scr_Ps::0><$Scr_H::2>`. Eles não são texto; o {{APP}} os retira. Um título recebe **Título 1** a **Título 6**, e um parágrafo com um dos estilos de parágrafo do Scrivener recebe o estilo do seu projeto com o mesmo nome (**Título**, **Citação em bloco**; o **Centered Text** do Scrivener vira **Centralizado**). Desde a 0.81.3, o texto com um dos estilos de caractere do Scrivener recebe o estilo de caractere do seu projeto com esse nome, e um estilo do Scrivener para o qual seu projeto não tem um estilo com esse nome vira um estilo do projeto, com a aparência que o Scrivener lhe dá. Projetos importados por versões anteriores à 0.14 mantinham os códigos como texto; o {{APP}} os retira quando tal projeto é aberto e avisa na barra de status. Os favoritos do projeto não eram importados antes da 0.14; para tê-los, importe o projeto do Scrivener de novo.

Se algo não veio corretamente, o projeto original permanece intocado e você pode copiar o texto diretamente do Scrivener.

### Trabalhando no formato do Scrivener

Em vez de fazer uma cópia, o {{APP}} também pode trabalhar diretamente em um projeto do Scrivener 3, para que você possa continuar no Scrivener depois (por exemplo em outro computador).

- **Abrindo**: **Arquivo > Abrir projeto…** (**Abrir outro…** na janela inicial) e escolha o arquivo *.scrivx* dentro da pasta *.scriv*. Na primeira vez, o {{APP}} pergunta: **Trabalhar no formato do Scrivener** (suas alterações são salvas no *.scriv*), **Converter…** (um projeto próprio do {{APP}} é criado a partir dele, como na importação acima) ou **Cancelar**. Depois disso, o projeto abre sem a pergunta, também por **Arquivo > Abrir recente**.
- **Novos projetos**: escolha **Formato: projeto do Scrivener 3 (.scriv)** na janela Novo projeto.
- **Salvando**: cada salvamento (também o automático) grava no *.scriv*. Só o que você alterou é gravado; os documentos que você não tocou mantêm os arquivos que o Scrivener gravou. O título da janela diz *(formato do Scrivener)*, e **Arquivo > Mostrar projeto no gerenciador de arquivos** e os backups usam a pasta *.scriv*.
- **Projetos grandes** (desde a 0.60.5): o salvamento automático enquanto você escreve coloca no *.scriv* apenas os textos, as notas e as sinopses que você alterou (e o Fichário, quando mudou), de modo que continua curto mesmo com centenas de milhares de palavras. Todo o resto, como o índice de busca do Scrivener, os instantâneos e as imagens, vem depois, quando você troca para outro programa, usa **Arquivo > Salvar** (**Ctrl+S**) ou fecha o projeto; portanto, antes de abrir o projeto no Scrivener, feche-o aqui ou troque antes para outro programa.
- **Um programa de cada vez**: se o Scrivener está com o projeto aberto (ou não foi fechado corretamente), o {{APP}} avisa e pergunta antes de abri-lo. Feche o projeto em um programa antes de abri-lo no outro. Quando o projeto foi alterado no Scrivener (ou por um serviço de sincronização) desde a última vez que você o teve aberto aqui, o {{APP}} o lê de novo e informa na barra de status quantos documentos mudaram; um documento alterado no Scrivener nesse meio-tempo nunca é sobrescrito com um texto mais antigo.

O que vai para o *.scriv*: o Fichário, os textos (como RTF, com suas imagens), as sinopses, as notas (também de imagens, PDFs, áudio e vídeo, desde a 0.81.3), as etiquetas, os status, os tipos de seção, **Incluir na compilação**, os favoritos (do projeto e de cada documento, também para endereços da web e arquivos), as metas do projeto com o prazo, as metas de documento, o histórico de escrita, o quadro de cortiça (a disposição de cada pasta e os lugares das fichas colocadas livremente), o título, o autor e o idioma do arquivo compilado (nas configurações de compilação do Scrivener), os instantâneos, os comentários e as notas de rodapé, as anotações e notas de rodapé no texto, os estilos de parágrafo e de caractere, as palavras-chave, os campos de metadados personalizados e seus valores, as coleções, as buscas salvas (coleções de busca, desde a 0.90.0), as imagens das fichas (desde a 0.90.0), imagens, PDFs, áudio e vídeo, páginas da web, ícones (pelos nomes do Scrivener; seus próprios ícones na pasta *Icons* dele) e mapas mentais (como um arquivo extra ao lado dos arquivos do documento, que o Scrivener deixa em paz). Tudo é gravado na forma que o Scrivener 3 para Windows usa:

- Palavras-chave, metadados personalizados e coleções mantêm os números e os nomes do Scrivener (desde a 0.81.2), de modo que são os mesmos nos dois programas.
- Estilos (desde a 0.81.3): os estilos de um texto que você altera são marcados nele como o Scrivener os marca, para que o Scrivener mostre os mesmos estilos. Um estilo que você criou no {{APP}} é adicionado aos estilos do projeto do Scrivener na primeira vez que um texto o usa, com sua aparência.
- Anotações e notas de rodapé no texto (desde a 0.81.3) continuam no texto, as anotações em sua cor (antes, um texto que você alterava as entregava ao Scrivener como comentários e notas de rodapé vinculados).
- Páginas da web (desde a 0.81.3): uma página que você salvou no {{APP}} entra como a própria página da web salva do Scrivener (um arquivo *.mht* com suas imagens) e mantém seu endereço. Outros arquivos para os quais o Scrivener não tem um tipo entram como documentos de texto que nomeiam o arquivo, com o arquivo ao lado; o {{APP}} os lê de volta como eram.
- Áudio, vídeo e páginas da web que versões anteriores à 0.81.2 transformavam em tais documentos de texto voltam a ser documentos de áudio, vídeo ou página da web do próprio Scrivener no próximo salvamento.
- Os textos (desde a 0.90.0) são gravados de volta com tudo o que o Scrivener guarda neles: recuos à direita abaixo de zero, listas com sua numeração, o sublinhado de links, imagens vinculadas a um arquivo, fontes nomeadas por seu tipo (como *Cambria-BoldItalic*) e o lugar de uma imagem cujos dados estão ausentes. Comentários e notas de rodapé mantêm seus IDs, se sua caixa está recolhida e sua formatação própria enquanto você não os altera.
- O quadro de cortiça (desde a 0.90.0): fichas que você coloca livremente mantêm seus lugares nos dois programas. O Scrivener guarda um lugar em larguras de ficha; o {{APP}} o converte em seu próprio tamanho de ficha, de modo que a disposição permanece a mesma, seja qual for o tamanho em que cada programa mostra as fichas. Cada pasta mantém sua disposição (livre ou em grade), como no Scrivener: escolher **Livre** ou a grade para o quadro de cortiça de uma pasta é lembrado para essa pasta.
- Etiquetas e status (desde a 0.90.0): a etiqueta e o status padrão de novos documentos, e os nomes que o Scrivener dá a "No Label" e "No Status", permanecem como o projeto os tem.
- Buscas salvas (desde a 0.90.0): uma busca salva do Scrivener é uma coleção de busca no {{APP}}, e uma coleção de busca que você cria ou altera vai para o *.scriv* como uma busca salva. A lista de resultados de busca recentes do Scrivener continua sendo só do Scrivener. Verificado com uma busca salva por qualquer uma das palavras; como o Scrivener nomeia os outros tipos (todas as palavras, a frase exata, uma expressão regular, apenas documentos incluídos ou deixados de fora da compilação) ainda não foi visto em um projeto real, de modo que uma busca salva desses tipos pode sair no Scrivener como uma busca por todas as palavras. Onde nos documentos uma busca procura não faz parte do que passa de um lado para o outro: uma busca salva lida do Scrivener procura em todos os lugares.
- A imagem de uma ficha (desde a 0.90.0): uma ficha que mostra uma imagem em vez de sua sinopse a mantém nos dois programas; tirá-la ou colocar uma no {{APP}} chega ao Scrivener.

O que fica apenas do lado do {{APP}} (em uma cópia de trabalho em *~/.local/share/{{APP}}/ScrivenerProjects*): formatos de roteiro, configurações de compilação (exceto o título, o autor e o idioma), objetos (seção 7) e o layout da janela; o que o *.scriv* já tiver disso é deixado nele como está.

Na primeira vez que um projeto do Scrivener é aberto ou importado, seu Fichário mostra as pastas que estavam abertas no Scrivener e os documentos ali selecionados (0.90.0; o {{APP}} nunca grava as configurações de janela do próprio Scrivener). Na primeira vez que você abre um projeto no formato do Scrivener com a 0.81.2 ou posterior, o {{APP}} lê o *.scriv* de novo e une as palavras-chave, os metadados personalizados e as coleções de sua cópia de trabalho com os do projeto, de modo que nada que você fez com uma versão mais antiga se perca; o salvamento seguinte os coloca no *.scriv*. A primeira abertura com a 0.90.0 ou posterior faz o mesmo para os favoritos, as metas de documento, o prazo, o histórico de escrita e o quadro de cortiça.

Se um salvamento no *.scriv* foi interrompido (uma falha, um disco cheio) depois de novos documentos terem sido gravados na cópia de trabalho, a abertura seguinte mantém esses documentos e avisa; o salvamento seguinte os coloca no *.scriv*. Documentos que foram excluídos no Scrivener permanecem excluídos.

**Arquivo > Salvar como…** (**Ctrl+Shift+S**) salva o projeto aberto com outro nome ou em outro lugar, e é assim que um projeto muda de formato. Na caixa de diálogo, digite o nome e escolha o lugar; o **tipo de arquivo** decide o formato: *Scrivener 3 Project (\*.scriv)* ou *{{APP}} Project (\*.qbx)* (um nome que termina em *.scriv* ou *.qbx* decide por si só). Um projeto é uma pasta: o programa a cria a partir do nome (*Nome.scriv*, ou *Nome.quillbinder* com *Nome.qbx* dentro). Um nome que já existe é recusado.

- Daí em diante a janela trabalha no novo projeto, e todo salvamento posterior vai para lá. O título do projeto é o nome que você digitou. Alterações que você ainda não tinha salvo vão apenas para o novo projeto.
- O original permanece no disco como estava naquele momento (guarde-o como cópia ou exclua-o); ele também está em **Arquivo > Abrir recente**, abaixo do novo projeto. Seu arquivo de bloqueio é removido quando a janela o deixa, e o novo projeto recebe o seu próprio.
- O novo projeto é um projeto próprio, com seus **backups** próprios: ele inicia uma nova série, e os backups feitos até agora ficam com o original (seção 14, Backups). Um projeto no formato do Scrivener recebe, para isso, um novo identificador em seu *.scrivx*.
- Salvar como funciona do mesmo jeito de um formato para o mesmo formato (um novo nome ou lugar) e para o outro (a troca que o menu "Formato do projeto" fazia até a 0.92).

**Arquivo > Exportar > Projeto do Scrivener…** grava uma **cópia** no formato do Scrivener, e a entrada abaixo dela no mesmo menu, **Projeto {{APP}}…**, uma cópia no formato próprio do {{APP}}; ambas deixam a janela no original: nada muda no projeto em que você trabalha (antes da 0.93.0 elas eram "Salvar uma cópia como ..." no menu "Formato do projeto").

### Exportando documentos

**Arquivo > Exportar > Arquivos…** salva os documentos selecionados para outros programas. Use a Compilação para um manuscrito pronto; use a Exportação para entregar documentos individuais a alguém ou para guardar uma cópia fora do {{APP}}.

1. No Fichário, selecione os documentos e as pastas a exportar.
2. Escolha **Arquivo > Exportar > Arquivos…**.
3. **Exportar arquivos de texto como**: Rich Text (.rtf), Word (.docx), Markdown (.md), texto simples (.txt), Fountain ou Final Draft (para roteiros, o Fountain vem pré-selecionado). Imagens, arquivos PDF, páginas da web, áudio e vídeo são exportados como estão; o formato conta então apenas para as notas deles.
4. **Metadados**: **Exportar metadados** grava um arquivo de texto ao lado de cada documento, "*nome* (metadata).txt", com seu título, tipo, etiqueta, status, palavras-chave, incluir na compilação, tipo de seção, contagem de palavras, meta, datas de criação e de alteração, metadados personalizados e favoritos. **Exportar sinopses** acrescenta a sinopse a ele (ou grava "*nome* (synopsis).txt" quando os metadados não são exportados).
5. **Opções**: **Incluir subdocumentos** (pastas viram pastas), **Exportar notas** ("*nome* (notes)" no formato do passo 3; texto simples para roteiros), **Exportar comentários e anotações no texto** (desmarque para entregar o texto limpo; as palavras comentadas permanecem, as notas de rodapé sempre permanecem), **Exportar instantâneos** (uma pasta "*nome* (snapshots)" com um arquivo por instantâneo, nomeado pela data e pelo título).
6. **Local**: a pasta onde salvar. Quando vários documentos estão selecionados, eles vão para uma nova pasta cujo nome você informa (o título do projeto é proposto; se a pasta já existe, um número é acrescentado).
7. Clique em **Exportar**. A pasta abre no gerenciador de arquivos.

As escolhas são lembradas para a próxima exportação.

O jeito rápido: **arraste documentos para fora do Fichário**, para o gerenciador de arquivos ou para a área de trabalho. Documentos de texto são salvos como arquivos RTF (ou Word, Markdown ou texto simples: **Arquivo > Opções > Comportamentos**), imagens e outros arquivos como estão, e uma pasta vira uma pasta com seus documentos dentro. Se o gerenciador de arquivos perguntar se deve mover ou copiar, qualquer um serve: os documentos permanecem no seu projeto.

### Comparar e transferir: um arquivo do Word ou do LibreOffice e o projeto

Quando um manuscrito volta de um editor, ou você continuou trabalhando nele no Word ou no LibreOffice, **Arquivo > Comparar e transferir > Comparar com arquivo…** encontra o que difere entre esse arquivo e o seu projeto e assume o lado que você escolher.

1. Escolha o arquivo: um arquivo do Word (*.docx*) ou do LibreOffice (*.odt*). Arquivos antigos do Word (*.doc*), RTF, texto simples e Markdown também podem ser comparados, mas apenas lidos: o texto deles pode ir para o projeto; o arquivo em si não pode ser alterado.
2. O {{APP}} compara o arquivo com **o que a Compilação faz do projeto**, não com o projeto inteiro: os documentos do grupo de compilação (o Rascunho, a menos que outro grupo seja escolhido na janela de compilação) que têm **Incluir na compilação**, na ordem do Fichário. Elementos pré e pós-textuais, a pasta Pesquisa e documentos deixados de fora da Compilação não contam. Se menos de 65 % do texto do arquivo é encontrado nele, o {{APP}} avisa e pergunta se deve comparar mesmo assim (**Comparar mesmo assim**); o arquivo pode pertencer a outro projeto, ou a Compilação deixa de fora o que o arquivo tem. Uma palavra conta como encontrada quando está em uma sequência de quatro palavras que o texto compilado também tem, de modo que algumas palavras alteradas não tornam um arquivo sem relação.
3. A janela **Comparar e transferir** mostra as diferenças. Escolha o lado que é atualizado:
   - **Atualizar o projeto a partir do arquivo**: as alterações são feitas nos seus documentos. Antes disso, cada documento alterado recebe um instantâneo, "Antes de Comparar e transferir (*nome do arquivo*)" (**Documentos > Instantâneos**), para que você sempre possa voltar. O texto que existe só no arquivo vira um novo documento (com o nome de seu título no arquivo), colocado depois do documento que vem antes dele.
   - **Atualizar o arquivo a partir do projeto**: o arquivo que você escolheu permanece como está. O {{APP}} grava uma cópia ao lado dele, "*nome* (Quillbinder).docx" (com um número quando esse nome já existe). Em um arquivo do Word, cada alteração é uma **alteração rastreada** do autor "Quillbinder": abra a cópia no Word ou no LibreOffice e aceite ou rejeite cada alteração ali (no Word, na guia Revisão; no LibreOffice, com Editar, Rastrear alterações, Gerenciar). Uma cópia do LibreOffice (*.odt*) recebe as alterações diretamente, sem marcas.
4. Na visualização, o texto **verde** entra, o texto **vermelho** riscado sai e o texto **azul** recebe o negrito, o itálico ou o sublinhado do outro lado. Parágrafos cinza são texto inalterado em volta de uma alteração, para mostrar onde ela está. Cada lugar tem um link **aceitar** ou **ignorar**; um clique o alterna.
5. Clique em **Transferir ... para o projeto** ou **Gravar ... em uma cópia do arquivo**. Uma mensagem informa o que foi feito; para o arquivo, **Abrir o arquivo** abre a cópia.

**Por capítulo** (**Arquivo > Comparar e transferir > Por capítulo**, também uma caixa de seleção na janela) lista os documentos à esquerda, cada um com uma caixa de seleção e o que difere ("2 lugar(es)", "só no arquivo", "não está no arquivo"). Desmarque um documento para deixá-lo como está; um clique mostra suas alterações à direita. Sem Por capítulo, todas as alterações são mostradas uma abaixo da outra.

**Passo a passo** (**Arquivo > Comparar e transferir > Passo a passo**, também na janela) mostra uma alteração de cada vez: **Aceitar** ou **Ignorar**, ou **Voltar**. No fim, a janela diz quantas alterações serão aceitas; então clique em Transferir. As duas opções são lembradas para a próxima vez.

O que é comparado: as palavras, e negrito, itálico e sublinhado. Cada parágrafo do arquivo é associado ao documento que tem suas palavras, de modo que uma cena também é encontrada onde foi movida no arquivo (ela é então marcada como "em outro lugar do arquivo"; a ordem do Fichário não é alterada). Títulos do arquivo que não são texto de um documento, como os títulos de capítulo criados pela Compilação, não são comparados.

O que vem marcado de início: toda alteração dentro de um documento. O texto que existe só em um dos lados vem marcado quando tem pelo menos 100 palavras (um novo capítulo do arquivo vira um novo documento; uma cena ausente do arquivo é adicionada à cópia). Um documento inteiro nunca é movido para a Lixeira, e uma parte inteira nunca é retirada do arquivo, a menos que você a marque.

Limites: parágrafos novos assumem a aparência do parágrafo vizinho. O texto em tabelas é comparado como os outros parágrafos (uma tabela em si nunca é retirada), e os campos contam com o texto que mostram. Cabeçalhos e rodapés, caixas de texto e o texto das notas de rodapé e dos comentários do arquivo não são comparados e permanecem como estão; comentários e notas de rodapé do projeto ficam no projeto e não são gravados no arquivo. Se o arquivo foi alterado depois da comparação, a cópia não é gravada; compare de novo.

### Imprimindo

**Arquivo > Imprimir documento atual…** (**Ctrl+P**) imprime o que o editor atual mostra:

- **Um texto** (um documento, ou vários no texto combinado): o texto com sua formatação e suas imagens, o título de cada documento acima de seu texto e as notas de rodapé no pé da página. Com o teclado em um suporte de cópia, é impresso o documento do suporte de cópia.
- **O quadro de cortiça**: as fichas na ordem mostrada, quantas couberem em uma folha (com linhas para recortar) ou uma ficha em cada folha, para papel de ficha.
- **A estrutura de tópicos**: as linhas e colunas como a estrutura de tópicos as mostra, com os títulos das colunas no alto de cada página.
- **Uma imagem** (tão grande quanto a página permitir), **um arquivo PDF** (suas páginas) ou **uma página da web** (seu texto).

A janela de impressão escolhe a impressora, as páginas e o número de cópias. **Seleção** imprime apenas o texto selecionado, ou apenas as fichas ou linhas da estrutura de tópicos selecionadas. Para salvar a impressão como um arquivo PDF, escolha **Imprimir em arquivo (PDF)** como impressora.

**Arquivo > Pré-visualização de impressão…** mostra primeiro as páginas; o botão de impressora da pré-visualização as imprime, **Fechar** (ou **Esc**) fecha a pré-visualização.

**Arquivo > Configurações de impressão…** decide como as coisas vão para o papel. A janela abre na aba do que o editor mostra; **Pré-visualização…** mostra o resultado antes de você clicar em **OK**. As configurações pertencem ao projeto; marque **Usar estas configurações também para novos projetos** para torná-las o padrão para novos projetos.

- **Página**: o tamanho e a orientação do papel (o botão **Papel da visualização de página** assume o papel e as margens de **Visualizar > Edição de texto > Tamanho da página e margens…**), as margens e o cabeçalho e o rodapé, cada um com uma parte esquerda, uma central e uma direita. Espaços reservados: `<$title>` (o que é impresso), `<$projecttitle>`, `<$author>`, `<$date>`, `<$p>` (o número da página) e `<$pages>` (o número de páginas). Desmarque **Cabeçalho e rodapé também na primeira página** para uma primeira página limpa.
- **Texto**: o que é impresso com o texto (o título de cada documento, a sinopse abaixo dele, as notas do documento depois do texto, comentários e anotações no texto como notas no pé da página, notas de rodapé no fim do texto em vez do pé de cada página, realces); as fontes (como no editor, ou **Uma fonte** à sua escolha); e o espaçamento entre linhas (como no editor, simples, 1,5 linha ou duplo, com espaço para correções). Documentos de roteiro mantêm seu layout de roteiro.
- **Fichas**: o tamanho da ficha (3 x 5 pol., 4 x 6 pol., 5 x 8 pol., A7, A6 ou um tamanho seu), quantas fichas couberem em uma folha ou uma ficha em cada folha, o que cada ficha mostra (título, sinopse, as primeiras linhas do texto quando não há sinopse, imagens, a cor da etiqueta como uma faixa no alto, status, palavras-chave, as linhas de uma ficha, uma linha em volta de cada ficha) e a fonte.
- **Estrutura de tópicos**: as sinopses (como a estrutura de tópicos as mostra, sempre ou nunca), também as linhas dentro de grupos fechados, os títulos das colunas no alto de cada página, uma linha entre as linhas, um ponto da cor da etiqueta e a fonte. Com muitas colunas, **Paisagem** na aba **Página** lhes dá mais espaço.

Imprimir documento atual imprime um documento como você trabalha nele. Para um manuscrito pronto, com a aparência de um formato de compilação, compile-o (seção 13) e imprima o arquivo PDF ou do Word.

---

## 15. Backups

O {{APP}} faz um backup (um arquivo ZIP do projeto inteiro) toda vez que você fecha um projeto e guarda os últimos cinco por projeto. Altere isso em **Arquivo > Opções > Backup**, onde você também pode escolher a pasta de backup (o melhor é em outra unidade, ou em uma pasta que seu programa de sincronização copie).

Desde a 0.91, os backups de todos os projetos ficam diretamente na pasta de backup, cada um com o nome de seu projeto, com a data e a hora. Desde a 0.92.0, o nome também leva o **ID** do projeto, cinco caracteres após um "#" (*My Novel #K7T2M bak 2026-11-20 15-40-00.zip*). O ID pertence ao projeto, não ao computador: é o mesmo em todo computador em que o projeto é aberto e permanece quando você renomeia o projeto ou move sua pasta. Os backups de um projeto são os arquivos que levam seu título e seu ID, e só eles são contados e limpos para ele, de modo que dois projetos com o mesmo título mantêm seus próprios backups e seu próprio número guardado. Uma cópia ou um novo projeto que o próprio programa faz é um projeto próprio, com seu próprio ID: **Arquivo > Salvar como…** e a entrada Projeto {{APP}} de **Arquivo > Exportar**, um projeto restaurado com **Restaurar de um backup** (ele também recebe um título próprio) e um projeto criado a partir de um modelo seu. Uma cópia feita fora do programa (uma pasta que você copiou à mão, um backup que você descompactou à mão) tem o ID de seu original: dê a tal cópia um título próprio. Um projeto no formato do Scrivener toma seu ID do identificador que o Scrivener guarda no *.scrivx*. Se sua pasta de backup está no OneDrive de uma organização, o administrador dela pode ter proibido "#" em nomes de arquivo; o OneDrive pessoal permite. Backups feitos por uma versão anterior à 0.92.0 não têm ID no nome: nunca são excluídos pela limpeza, **Restaurar de um backup** ainda os lista, e você mesmo os exclui quando não precisar mais deles. Quando você altera o título de um projeto, os backups feitos sob o título antigo permanecem até você excluí-los. As versões 0.70 a 0.90 guardavam os backups de cada projeto em uma pasta própria dentro da pasta de backup (*My Novel 1a2b3c4d*); tal pasta é deixada como está, nada mais é acrescentado nem excluído ali, e **Restaurar de um backup** ainda lista seus backups. Um backup roda ao lado da janela: depois de um momento, uma janela de progresso mostra em que ponto está e pode cancelá-lo. Quem está com o projeto aberto (o arquivo de bloqueio com o nome do computador e do login) não entra no backup.

- Com **Fazer backup a cada salvamento manual** (**Arquivo > Opções > Backup**), cada **Arquivo > Salvar** (**Ctrl+S**) também faz um backup. Dois backups dentro do mesmo segundo recebem um número ("... (2)"). Quando esse backup não pode ser feito, está incompleto ou precisa esperar fora da pasta de backup, um aviso ao lado do texto informa (0.92.0; antes, só a barra de status o fazia, por alguns segundos). Seu texto é salvo de qualquer modo. O aviso vem uma vez por causa até você sair do programa; a barra de status o informa a cada salvamento.
- **A linha de mensagens** sob o Fichário mostra mensagens curtas uma de cada vez, por 5 segundos (10 com um link) ou até você clicar nela; as outras esperam. Os complementos também a usam (por exemplo "Modelo descarregado, uso de RAM mínimo" quando Assistente de tradução ou Transcrição de fala liberou seu modelo após 15 minutos sem uso; o tempo está nas opções deles). **Ajuda > Solução de problemas e desenvolvimento > Informações do sistema** nomeia a memória que o programa mantém.
- **Complementos que não estão instalados.** **Editar > Ferramentas de escrita** lista "Assistente de tradução" e "Transcrição de fala" enquanto o complemento está ausente ou desativado; cada um mostra um aviso com um botão que abre **Ajuda > Complementos e atualizações > Gerenciador de complementos**.
- **Arquivo > Backup > Fazer backup agora** faz um backup imediatamente. Desde a 0.94.0, o sucesso é informado na linha de mensagens sob o Fichário ("Backup salvo." com o link **Mostrar pasta**, que abre a pasta do backup; a dica nomeia o arquivo; uma mensagem com um link permanece 10 segundos); nenhuma caixa espera por OK. Uma caixa só aparece quando algo falhou, ou quando o backup é feito mas alguns arquivos não puderam ser lidos ou um backup mais antigo não pôde ser excluído.
- **Um backup que um desligamento ou um comando kill impediu de ser feito** é anotado e feito na abertura seguinte (0.81). Desde a 0.93.0, a anotação permanece até que esse backup seja feito (na pasta de backup, ou esperando ao lado dela como descrito acima): uma abertura cujo backup falha deixa a anotação para a abertura seguinte. **Um backup mais antigo que a limpeza não consegue excluir** (um arquivo bloqueado ou protegido) permanece, e uma mensagem nomeia o arquivo e o motivo uma vez; exclua-o à mão. Enquanto você continua escrevendo, essa mensagem é um pequeno aviso ao lado do texto que não toma nenhuma tecla.
- **Fazer backup em…** salva um backup onde você escolher, por exemplo em um pendrive.
- **Restaurar de um backup…** (0.70) lista os backups do projeto aberto com sua data, tamanho e se estão completos; escolha um e **Restaurar e abrir**: ele é descompactado em uma nova pasta ao lado do projeto, *… (restored 2026-11-10 14-03)*, seu título diz "(restaurado …)" e ele abre ali. O projeto que você tinha aberto não é alterado. A lista contém os backups com o ID deste projeto, os sem ID (feitos antes da 0.92.0) e os de outro projeto com o mesmo título; o nome do arquivo na lista mostra qual é qual, de modo que nada fica fora de alcance. Desde a 0.93.0, ela também lista os backups que esperam na pasta de trabalho do próprio programa (feitos enquanto a pasta de backup não podia ser alcançada; o backup seguinte na pasta de backup os coloca lá): eles são marcados como "esperando (ainda não está na pasta de backup)" e são restaurados do mesmo modo. Desde a 0.81, uma restauração de qualquer tamanho funciona (backups de 2 GB ou mais, arquivos de mais de 1 GB, como vídeos, mais de 65.535 arquivos), cada arquivo é verificado conforme é gravado, uma restauração que não cabe na unidade não é iniciada, uma janela de progresso pode cancelá-la e, quando arquivos não podem ser restaurados, a mensagem nomeia cada um deles com o motivo (disco cheio, entrada danificada). Desde a 0.93.0, a mensagem tem três formas: **restaurado** (tudo está lá), **restaurado de forma incompleta** (assim intitulada, com um sinal de aviso, com os arquivos que faltam nomeados com o motivo: olhe o projeto restaurado antes de confiar nele) e **não restaurado**; e quando o título do projeto restaurado não pôde ser marcado como "(restaurado …)", a mensagem avisa.
- **Mover backups antigos para a pasta de backup…** (0.93) move os backups que as versões 0.70 a 0.90 guardavam em uma pasta própria dentro da pasta de backup (*My Novel 1a2b3c4d*) para a própria pasta de backup, sob seus nomes, de modo que fiquem junto com os outros. Ele só fica ativado enquanto o projeto aberto tem uma pasta assim com backups. Ele pergunta antes e diz quantos há; um backup cujo nome já existe na pasta de backup nunca é sobrescrito e permanece onde estava; o relatório informa o que foi movido e o que ficou (com o motivo), e a pasta antiga é removida quando nada resta nela. Esses backups não têm ID no nome, de modo que a limpeza nunca os conta nem os exclui, como antes; **Restaurar de um backup** os lista em qualquer um dos dois lugares.
- **Mostrar pasta de backup** abre a pasta de backup.

**Onde um backup é montado (desde a 0.90.3).** Um backup é empacotado e verificado em uma pasta do próprio programa, *~/.local/share/{{APP}}/backup-work* (no Windows, *data\backup-work* na pasta do programa), e só o arquivo pronto é colocado na pasta de backup, em um passo e sob seu nome final: movido para lá quando as duas pastas estão na mesma unidade, ou então copiado diretamente para seu nome final e comparado com o original. Assim, uma pasta de backup que um programa de sincronização observa, ou que está em um pendrive ou em uma unidade de rede, nunca contém um backup pela metade nem um arquivo temporário, e um backup que foi interrompido nunca parece um backup concluído. (Antes da 0.90.3, o backup era gravado dentro da pasta de backup como *… .zip.part* e renomeado ali.) A pasta de trabalho fica vazia entre os backups, exceto por uma pequena anotação da pasta de backup a que serve.

**Quando a pasta de backup está fora de alcance** (uma unidade que não está conectada, uma pasta que não pode ser criada nem gravada, falta de espaço), o backup é feito mesmo assim e espera na pasta de trabalho. Uma mensagem avisa e nomeia as duas pastas, e o backup seguinte do projeto coloca os backups em espera no lugar antes do seu próprio. Esperam tantos quantos backups são mantidos. **Fazer backup em…** em uma pasta que não pode ser gravada apenas avisa, com o motivo; escolha outra pasta.

**Cópias feitas por um programa de sincronização.** Só os arquivos nomeados como o {{APP}} nomeia seus backups (*Título #K7T2M bak 2026-11-20 07-56-13.zip*, com "(2)" ou "(incomplete)" onde couber) contam como backups e podem ser excluídos pela limpeza. Outros arquivos na pasta de backup de um projeto são deixados em paz e não contam: por exemplo, as cópias que um programa de sincronização faz quando tem problemas com um arquivo (*… -MyComputer-safeBackup-0001.zip*, *… (conflicted copy).zip*) ou uma cópia que você fez à mão. (Antes da 0.90.3, tais cópias contavam como backups de sua data, e várias delas podiam empurrar para fora os verdadeiros backups mais antigos.) **Restaurar de um backup…** ainda as lista, para que você possa restaurar a partir de uma cópia; exclua você mesmo as cópias de que não precisa.

Projetos grandes (mais de 4 GB, por exemplo com áudio e vídeo) também têm backup correto (desde a versão 0.20). Um arquivo que não pode ser lido (por exemplo por causa de suas permissões, ou um arquivo de uma pasta sincronizada que ainda não está neste computador) é deixado de fora; o backup ainda é feito, mas se chama *… (incomplete).zip*, uma mensagem nomeia os arquivos que faltam (também para os backups automáticos quando um projeto abre ou fecha, 0.65), e nenhum backup mais antigo é excluído por causa dele: o número de backups a manter conta apenas backups completos. Quais backups são os mais antigos é dito pela data e pela hora em seus nomes, de modo que renomear o projeto não muda nada quanto a quais são mantidos (0.81; antes, um novo título que ordenava antes do antigo fazia cada novo backup contar como o mais antigo, e ele era excluído na hora). Depois da limpeza, o programa verifica se o novo backup ainda está lá. Dos backups incompletos, os mais novos até o número a manter permanecem (0.81). Uma pasta do projeto que é um link para uma pasta em outro lugar tem backup com seus arquivos (0.81); um link que não aponta para lugar nenhum, ou de volta para dentro do projeto, torna o backup incompleto. Se o programa é encerrado por um sinal (um desligamento que não espera, um comando kill) enquanto **Fazer um backup quando um projeto é fechado** está ativado, não há tempo para o backup; na próxima vez que o projeto abre, o backup é feito e a barra de status avisa (0.81). O projeto é salvo antes de cada backup; se esse salvamento falha (disco cheio, pasta que não pode ser gravada), nenhum backup é feito e uma mensagem diz o motivo, de modo que um backup nunca fica sem suas últimas alterações sem que você saiba. O backup de um projeto no formato do Scrivener contém o *.scriv* e, ao lado dele, a cópia de trabalho do {{APP}} (*… - working copy of {{APP}}*), com as palavras-chave, os estilos, os objetos e outras coisas que só o {{APP}} conhece.

**Para restaurar um backup à mão** (por exemplo em outro computador): abra a pasta de backup, descompacte o arquivo ZIP (clique com o botão direito > Extrair) e abra o projeto que está nele com **Arquivo > Abrir projeto**.

---

## 16. Opções

**Arquivo > Opções** (**Ctrl+,**) tem uma fileira de ícones no alto para as seções, uma lista de páginas (ou abas) à esquerda e as opções à direita. Nada muda até você clicar em **OK** ou **Aplicar**. No canto inferior esquerdo, **Gerenciar** exporta o perfil em uso para um arquivo e importa um (**Exportar perfil...**, **Importar perfil...**, por exemplo para levar suas configurações para outro computador; um arquivo de opções mais antigo (*.qboptions*) pode ser importado como perfil; veja "Perfis" abaixo), abre **Gerenciar perfis...**, salva a aparência atual como um arquivo de tema (um arquivo existente só é substituído quando o novo foi gravado por completo; se a gravação falha, o arquivo antigo permanece como estava e a mensagem avisa) e lista os temas; **Padrões** devolve todas as opções ao padrão (seus projetos recentes, seus próprios temas, layouts, barras de ferramentas e atalhos permanecem).

**Onde cada opção é guardada.** Aponte para uma opção para ver onde seu valor é guardado: sua dica diz se o projeto aberto a guarda ou se o programa a guarda para todos os projetos, e se esse lugar é **fixo** (uma configuração que só o programa pode guardar, como o idioma) ou **depende da configuração** **Configurações para** no alto (e, na página Aparência, **Mostrar e alterar a aparência de**). (A 0.90.1 mostrava um P verde ou um A azul antes de cada opção; a 0.90.2 deixa isso para a dica.) Em uma tela pequena, as páginas rolam, de modo que OK, Cancelar e Aplicar permanecem à vista.

Desde a 0.70, a janela abre para **Todos os projetos** (as configurações do programa); escolha **Este projeto** no alto para definir algo apenas para o projeto aberto. As configurações próprias de um projeto podem tornar o salvamento mais seguro, mas nunca menos seguro: vale o menor atraso de salvamento automático, um backup que o programa faz permanece ativado, o maior número de backups é mantido e a saída automática é exclusiva do programa. O último salvamento vem no máximo 10 segundos depois de uma alteração, seja qual for o atraso do salvamento automático.

- **Geral**: **Inicialização** (reabrir o último projeto, as caixas de diálogo de arquivos da área de trabalho, o **Idioma** do programa, a partir da próxima inicialização: English, Deutsch, Español, Français, Português (Brasil), Italiano, Русский, Bahasa Indonesia, Türkçe, Polski ou Nederlands (os nove novos são traduções automáticas, assim marcadas na lista até que um falante as confira; o manual, o guia de início rápido e o projeto de boas-vindas acompanham o idioma, em inglês onde um idioma não os tem; na primeira inicialização é adotado o idioma do sistema quando ele é um destes, e a página da primeira inicialização oferece a mesma lista; **Agrupar os dígitos de números grandes**: contagens de palavras, metas e estatísticas com um separador entre os milhares, como seu sistema escreve os números (44,825 ou 44.825), ou 44825 quando desmarcado); **Salvando** (intervalo do salvamento automático, o número de projetos recentes, instantâneos a cada Arquivo > Salvar); **Informações do autor** (seu nome, endereço, telefone e e-mail para folhas de rosto, usados pelos espaços reservados `<$author>`, `<$street>`, `<$city>`, `<$state>`, `<$zip>`, `<$country>`, `<$phone>`, `<$email>`, `<$address>`); as opções das ferramentas de escrita (**Bloco de rascunho**); **Modelos compartilhados** (outra pasta de modelos de projeto, por exemplo uma sincronizada entre seus computadores; seus modelos aparecem em Arquivo > Novo projeto); **Citações** (o programa que **Formatar > Bibliografia/citações...** inicia, por exemplo Zotero ou JabRef); **Separadores** (o que vai entre documentos mesclados e entre notas do bloco de rascunho adicionadas a um documento: uma única quebra de linha, uma linha em branco, uma quebra de página, nada ou um texto seu); **Saída automática** (salvar e fechar após um número de minutos sem entrada de teclado ou mouse); **Avisos** (**Mostrar console de registro interno** mostra as mensagens do programa desta sessão, **Redefinir todos os avisos** volta a fazer as perguntas respondidas com "Não perguntar novamente").
- **Edição**: **Opções** (recarregar o último zoom dos editores, o zoom de texto padrão de cada área, centímetros ou polegadas e o passo da régua, a tecla Insert para sobrescrever, um formato de data seu para Inserir > Data atual, o que a contagem ao vivo mostra, anotações no texto sem cor, comentários e notas de rodapé que param antes da pontuação, um ponto de inserção em bloco); **Formatação** (a formatação e os estilos de novos projetos); **Revisões** (as cinco cores de Formatar > Modo de revisão, da paleta compartilhada).
- **Comportamentos**: **Modo de composição** (tela, janela principal fora do caminho, Esc); **Links de documentos** (onde os links de documentos abrem, links de retorno); **Clique duplo** (um clique duplo no quadro de cortiça cria uma ficha); **Arrastar e soltar** (documentos soltos sobre documentos de texto, o formato do texto arrastado para outros programas); **Pastas e arquivos** (pastas abrem como um quadro de cortiça; um texto com subdocumentos abre como uma pasta); **Reprodução** de áudio e vídeo (seção 6); **Referência rápida** (painéis reabrem com seu projeto); **Tecla Return** (Return encerra a edição de uma sinopse; Return cria uma nova ficha ou linha); **Instantâneos** (uma mensagem quando instantâneos são feitos).
- **Aparência**: o tema e onze partes da janela (Interface geral, Fichário, Modo de composição, Quadro de cortiça, Fichas, Inspetor e notas, Editor principal, Estrutura de tópicos, Texto combinado, Barras de progresso das metas, Marcas textuais), cada uma com suas opções, fontes e cores, e **Cores padrão** (desde 0.95: as seis etiquetas com que os novos projetos começam, as seis cores da paleta de cor do texto e da paleta de realce e as cores com que os dois botões começam; **Padrões** devolve o padrão do programa; os projetos existentes mantêm as suas etiquetas). Veja "Temas, cores e fontes" abaixo. **Interface geral** também tem o caminho completo do projeto na barra de título e o tamanho dos ícones da barra de ferramentas.
- **Correções**: correções durante a digitação (seção 4) e **Ortografia** (o dicionário, palavras com números, o sublinhado, a lista pessoal de palavras).
- **Teclado**: o atalho de cada comando (seção 19).
- **Compartilhamento**: **Importar** (comentários, notas de rodapé, estilos, a codificação de texto simples, arquivos HTML; seção 14), **Exportar** (o formato de arquivo e as notas de Arquivo > Exportar > Arquivos), **Conversão** (como os arquivos *.doc* são lidos).
- **Backup**: quando fazer backup (ao abrir, ao fechar, a cada Arquivo > Salvar), quantos backups manter (uma instalação nova começa com os 25 mais recentes; o campo contém só o número, e as palavras ficam ao redor dele: "Manter os mais recentes [25] backups de cada projeto") e onde.
- **Extras**, para as adições próprias do {{APP}}: **Mapa mental** (as opções dos quadros, o formato e a aparência de novas notas, o fundo de quadros sem cor própria em temas claros e escuros, o tamanho dos ícones em notas e linhas), **Objetos** (se novos projetos recebem seus próprios modelos de objeto ou os padrão), **Complementos e plugins** (os formatos de compilação e as ferramentas de escrita instalados, as pastas de plugins), **Relatórios de falhas** (após quantos segundos sem resposta um relatório de travamento é gravado, quantos relatórios são mantidos).
- **Fala** (com o complemento Transcrição de fala): o modelo, o dispositivo, os idiomas, o microfone e as pausas (veja a seção 10, *Transcrição de fala*). Ferramentas de escrita de complementos posteriores também podem trazer uma seção própria.

Outras janelas do programa podem abrir as opções em uma página, por exemplo a página Reprodução a partir do reprodutor de mídia.

Atualizações e complementos têm suas próprias janelas: **Ajuda > Complementos e atualizações > Instalar atualização ou complemento…** e **Ajuda > Complementos e atualizações > Gerenciador de complementos…** (seção 17).

As configurações específicas do projeto (etiquetas, status, tipos de seção, a aparência do texto, metadados personalizados, pastas especiais, título e autor para compilar) ficam em **Projeto > Configurações do projeto**.

### Configurações para um projeto

No alto de **Arquivo > Opções**, **Configurações para:** escolhe onde fica guardado o que você altera:

- **Todos os projetos (o programa)**: as configurações do programa, para todo projeto. É aqui que a janela abre.
- **Este projeto**: o projeto aberto guarda o que você altera aqui como uma configuração própria (em *settings/options.ini* de sua pasta). Ela vale enquanto esse projeto está aberto; o que o projeto não define vem das configurações do programa. O título da janela diz "Opções: Este projeto", e a linha sob a escolha diz quantas configurações próprias o projeto tem (clique no número para vê-las). **Usar configurações do programa** remove todas elas; as configurações do programa voltam a valer para o projeto. Uma alteração feita fora das opções, por exemplo um atalho alterado na janela do teclado ou uma barra de ferramentas reorganizada, vai para o projeto quando o projeto já tem um valor próprio para ela, e para o programa caso contrário.
Algumas configurações pertencem somente ao programa e não são mostradas para um projeto: os projetos recentes, reabrir o último projeto, as caixas de diálogo de arquivos da área de trabalho, as posições das janelas e os layouts, os tempos dos relatórios de falhas e a aparência para novos projetos. A aparência de um projeto sempre foi só dele: a página **Aparência** mostra a aparência do projeto aberto ou a aparência para novos projetos (**Mostrar e alterar a aparência de**, no alto); com **Este projeto** escolhido acima, ela mostra apenas a aparência do projeto. **Padrões** e **Gerenciar > Importar perfil…** atuam somente sobre as configurações do programa, portanto escolha **Todos os projetos** para eles.

**Quais configurações vêm primeiro** (**Geral > Inicialização**, para todos os projetos): marcada (o padrão), as configurações próprias de um projeto vêm primeiro e o programa preenche o resto. Desmarcada, as configurações do programa sempre valem: as configurações próprias dos projetos não são usadas (elas permanecem nos projetos para mais tarde), e todo projeto mostra a aparência para novos projetos.

**Projeto > Salvar configurações do projeto em arquivo…** inclui as opções próprias do projeto (a seção **Opções deste projeto**), para que possam ser levadas a outro projeto.

### Temas, cores e fontes

Um tema define as cores e, se ele quiser, as fontes de cada área da janela. Escolha um em **Visualizar > Temas** ou com o botão de tema da barra de ferramentas Temas (ative a barra em **Visualizar > Barras de ferramentas**); a alteração vale na hora, sem reiniciar. Nove temas são embutidos:

| Tema | Aparência |
|-------|------|
| Tema escuro da Shelin | janela e Fichário escuros, um papel quente com texto preto, Cinzel nos títulos do Texto combinado (desde 0.95; fica no alto da lista e pode ser excluído em Gerenciar temas; depois disso não volta com as atualizações) |
| Padrão (seguir a área de trabalho) | as cores da sua área de trabalho (o esquema de cores do KDE) |
| Claro | janela cinza-clara, papel branco |
| Modo escuro | cinza-escuro, com fichas escuras |
| Sépia | creme quente e marrom |
| Ardósia | janela cinza, papel branco sobre uma mesa cinza |
| Meia-noite | azul-escuro com um destaque âmbar |
| Oceano | azul-esverdeado claro |
| Alto contraste | branco e amarelo sobre preto |

Complementos podem trazer mais temas (o complemento "Extra themes" tem Northern Night, Forest, Graphite, Warm Paper e Rose Dawn).

**O botão de tema** (0.90.2, alterado na 0.91 e na 0.94). Um botão, **Tema**, na barra de ferramentas Temas, que no início está desativada (**Visualizar > Barras de ferramentas > Temas** a mostra; antes da 0.94, quatro botões e uma lista ocupavam um terço da barra de ferramentas principal). Seu menu lista:

- **Claro** e **Escuro**: um clique mostra esse tema na hora, como ele é.
- **Meu visual 1** e **Meu visual 2** guardam dois visuais seus. Escolher um vazio (o menu diz *vazio*) salva nele o visual que você vê agora (o tema com todas as cores e fontes que você alterou por cima); escolher um preenchido traz seu visual de volta. **Salvar o visual atual como Meu visual 1** e **...como Meu visual 2**, no fim do menu, salvam ali o visual que você vê agora (eles perguntam antes de substituir um visual). Os dois visuais são temas comuns: estão na lista abaixo e em **Visualizar > Temas**, onde também podem ser exportados ou excluídos (um excluído deixa sua entrada vazia).
- **Todos os temas**, abaixo, com o que está em uso marcado. Escolher um faz o que **Visualizar > Temas** faz: o projeto aberto e os novos projetos o usam, e a pergunta sobre manter ou não as suas alterações por cima vem como antes.
- **Seu visual nunca se perde por uma escolha.** O menu mostra um tema como ele é, de modo que o que você vê realmente muda. Se cores ou fontes suas estão por cima do tema mostrado, elas se perderiam com isso, então são salvas antes: em um Meu visual vazio sem pergunta (a barra de status diz onde) e, quando os dois contêm um visual, o {{APP}} pergunta se deve salvar em Meu visual 1, em Meu visual 2 ou trocar sem salvar.

Como todo controle de barra de ferramentas, o botão de tema pode ser movido ou retirado em **Visualizar > Barras de ferramentas > Personalizar barras de ferramentas**, e a barra de ferramentas Temas inteira pode ser ativada ou desativada em **Visualizar > Barras de ferramentas**.

**A página em temas escuros** (0.70). A página padrão do {{APP}} é um papel verde-oliva claro ao lado de uma mesa cinza. Com um tema escuro, a página é um cinza muito escuro com texto branco (ou a própria página escura do tema, se ele tiver uma), de modo que um tema escuro não deixa uma página brilhante em uma moldura escura. Uma cor de página que você escolheu permanece em todos os temas (clique com o botão direito > Aparência, ou Arquivo > Opções > Aparência).

**Alterando cores e fontes.** **Arquivo > Opções > Aparência** (ou **Visualizar > Temas > Cores e fontes…**) lista à esquerda as partes da janela. Cada uma mostra suas opções, suas fontes e suas cores (um clique em uma cor mostra a paleta compartilhada: as cores das etiquetas, as seis cores padrão e **Outra...**):

| Parte | Fontes | Cores |
|------|-------|--------|
| Interface geral | menus, botões e painéis | janela, Fundo do inspetor (as listas e os campos de texto do inspetor), linhas alternadas, texto, botões, seleção, links, dicas, bordas, texto esmaecido |
| Fichário | Fichário | fundo, títulos |
| Modo de composição | (nenhuma) | fundo, papel, texto |
| Quadro de cortiça | (nenhuma) | quadro de cortiça, carimbo de status |
| Fichas | títulos das fichas, texto das fichas | ficha, título, texto, linha sob o título, linhas pautadas |
| Inspetor e notas | sinopse | Faixas do inspetor (a fileira de abas, os títulos, a linha de Etiqueta e Status), etiquetas, ficha e texto da sinopse, notas e seu texto |
| Editor principal | título da barra de cabeçalho | papel, texto, o espaço ao lado da coluna de texto, a mesa em volta das páginas (mostrada apenas na visualização de página), a barra de cabeçalho (normal, editor ativo, travado no lugar) |
| Estrutura de tópicos | estrutura de tópicos | fundo, linhas alternadas, texto |
| Texto combinado | títulos no texto combinado | (nenhuma) |
| Barras de progresso das metas | (nenhuma) | sessão, rascunho, meta atingida |
| Marcas textuais | (nenhuma) | links no texto, resultados de busca, sublinhado de ortografia, as marcas de Mostrar invisíveis |

**Cada projeto guarda a sua própria aparência.** O tema do Fichário, do editor, do quadro de cortiça, da estrutura de tópicos, do inspetor e do modo de composição, suas próprias cores e fontes para eles, a largura da coluna de texto e a borda ao lado do texto pertencem ao projeto: alterá-los em um projeto deixa os outros como estão, e uma atualização do {{APP}} nunca muda a aparência de um projeto existente. A lista **Mostrar e alterar a aparência de**, no alto de **Arquivo > Opções > Aparência**, escolhe de quem é a aparência que a página mostra e altera: **Este projeto** (o aberto; escolhido primeiro) ou **Novos projetos** (a aparência com que um novo projeto começa). Alternar entre os dois mantém o que você alterou em cada um até você pressionar **Aplicar** ou **OK**. As cores dos menus e painéis (**Interface geral**) e a fonte dos menus, botões e painéis são as mesmas em todos os projetos. Um projeto aberto pela primeira vez pela versão 0.40.2 ou posterior mantém a aparência que tinha até então. **Visualizar > Temas** define o tema do projeto aberto e dos novos projetos ao mesmo tempo.

**Menus de clique com o botão direito.** Onde quer que você clique com o botão direito, o menu tem um submenu **Aparência** com as cores e fontes do que está sob o mouse: no texto e no papel ao lado dele, o papel, o texto, o espaço ao lado da coluna, os links, os resultados de busca, o sublinhado de ortografia, a largura da coluna e a borda; na barra de cabeçalho acima de um editor, suas cores e fonte; no Fichário, seu fundo, títulos, fonte, recuo e cores das etiquetas; no quadro de cortiça, a cortiça, as fichas e suas fontes, os carimbos de status, a largura das fichas; na estrutura de tópicos, seu fundo, linhas, texto e fonte; no inspetor e em suas notas, as faixas, as etiquetas, a ficha da sinopse e as notas, e o Fundo do inspetor, que é o mesmo em todos os projetos; no modo de composição, seu fundo, papel e texto. Uma cor mostra a paleta compartilhada e **A cor do tema**; **Opções de aparência...** abre esta página das opções. Como as opções, esses menus alteram a aparência do projeto aberto (a primeira linha do submenu diz isso) ou, sem um projeto, a aparência dos novos projetos.

Clique em uma cor para alterá-la, em um botão de fonte para escolher uma fonte. A pequena imagem no alto mostra o resultado antes de você pressionar **Aplicar** ou **OK**. Suas alterações ficam por cima do tema: uma entrada alterada é mostrada em negrito, o botão **↺** ao lado dela a devolve ao tema, e **Redefinir cores e fontes para o tema** faz isso para todas elas. Uma cor que você não alterou e que o tema não define acompanha as outras; por exemplo, o Fichário usa a cor das listas até você lhe dar uma cor própria. O **Fundo do inspetor** colore apenas as listas e os campos de texto do inspetor; os campos das barras de ferramentas, a régua e as listas das opções têm a cor da janela e dos menus.

A fonte e o tamanho do texto que você escreve não fazem parte do tema: pertencem ao projeto (**Projeto > Configurações do projeto > Formatação**, seção 4).

**Temas seus.** **Salvar como tema…** nas opções (ou **Visualizar > Temas > Salvar aparência atual como tema…**) guarda o tema junto com as suas alterações sob um novo nome. Ele aparece então em **Visualizar > Temas** como os outros. **Visualizar > Temas > Gerenciar temas…** lista todos os temas: **Usar este tema**, **Duplicar…** (também para temas embutidos, para obter um ponto de partida seu), **Renomear…** e **Excluir** (apenas seus próprios temas), **Exportar…** (grava um arquivo *.qbtheme* que você pode repassar) e **Importar…** (adiciona tal arquivo).

Quando você escolhe outro tema tendo alterações suas, o {{APP}} pergunta se deve mantê-las com o novo tema ou usar o tema como ele é.

Os links no seu texto são guardados com um único azul; em papel escuro, são mostrados em um azul mais claro (**Editor principal > Links no texto**). O texto que você colorou mantém sua cor em todos os temas.


### Perfis

Um tema é só uma aparência: cores e fontes. Um **perfil** (desde 0.95) guarda tudo o mais das configurações do programa: o idioma, todas as opções, as barras de ferramentas e suas posições, o tamanho e a posição da janela, os painéis mostrados, os divisores, o conjunto de teclado e seus atalhos, os backups e a pasta de backup, as verificações de atualizações, as cores padrão, os layouts salvos de **Visualizar > Layout**, e ele nomeia o seu tema, de modo que trocar o perfil troca a aparência também. Seu nome e endereço, o nome deste computador, os projetos recentes e as posições do projeto de boas-vindas não fazem parte de um perfil.

- **Trocar.** **Visualizar > Perfis** lista os perfis; um clique troca na hora, sem reiniciar (o idioma é a única coisa que precisa de reinício; o programa avisa). O perfil que você deixa é salvo antes. A barra de ferramentas **Perfis** (**Visualizar > Barras de ferramentas > Perfis**, desativada no início) mostra o perfil em uso; um clique no botão dela lista os perfis. As configurações próprias de um projeto (**Configurações para: Este projeto**) continuam valendo por cima do perfil. O projeto aberto mostra a aparência do novo perfil, como quando você escolhe um tema em **Visualizar > Temas**.
- **Padrão.** Um programa que já foi usado antes adota suas configurações como o perfil **Padrão** quando é atualizado para 0.95, para que nada se perca. Uma instalação nova começa com as configurações padrão do programa, no tema claro ou no escuro conforme o seu sistema está configurado, onde o sistema informa isso. A página da primeira inicialização preenche **Padrão**. O **Padrão** não pode ser excluído.
- **Um novo perfil** a partir das configurações como estão agora: **Visualizar > Perfis > Salvar configurações como novo perfil…** (ou **Gerenciar perfis… > Salvar configurações como novo perfil…**). O novo perfil passa a ser o que está em uso. **Gerenciar perfis…** também renomeia, copia e exclui perfis (o perfil em uso não pode ser excluído; troque antes).
- **Shelin** é um perfil que vem com o programa. Ele fica no alto da lista, define Sitka para o texto novo e o seu próprio tema, e pode ser excluído; depois disso não volta com as atualizações. As fontes que ele nomeia e que não estão instaladas (Cinzel, Sitka) são substituídas por outras. A escolha dele do mecanismo de reprodução precisa do complemento Formatos extras de vídeo e áudio; as escolhas de modelos das ferramentas de escrita valem apenas onde esses complementos e modelos estão instalados.
- **Tamanhos em outra tela.** As larguras do Fichário, do editor e do inspetor são guardadas como proporções da janela; um perfil feito em uma tela grande mostra as mesmas proporções em uma tela menor ou em um tablet (as colunas laterais mantêm uma largura legível), e a janela é trazida para dentro da tela.
- **Exportar e importar.** **Gerenciar perfis… > Exportar…** (ou **Arquivo > Opções > Gerenciar > Exportar perfil…**) grava o perfil em um único arquivo (*.qbprofile*) com os seus layouts e, se for um dos seus, o seu tema. **Importar…** adiciona um arquivo assim como um novo perfil (não se troca para ele). Um arquivo pode nomear programas, pastas, arquivos ou endereços; como em outros arquivos que vêm de outra pessoa, você é consultado antes que eles sejam aceitos (uma pasta que não existe neste computador não teria utilidade). Um perfil que não foi salvo por este programa neste computador (um arquivo copiado ou sincronizado para a pasta de perfis) pergunta da mesma forma sobre seus programas, pastas, arquivos e endereços quando você troca para ele. Um arquivo de opções mais antigo (*.qboptions*) também pode ser importado como perfil.

**Projeto > Salvar configurações do projeto em arquivo…** continua como era.

---

## 17. Atualizações e complementos

**Ajuda > Solução de problemas e desenvolvimento > Guia do desenvolvedor: atualizações e complementos** explica, para quem constrói o {{APP}} ou escreve complementos, como funcionam as atualizações e os complementos e quais ferramentas os criam; ele também contém o que um amigo com inclinação técnica precisa para configurar as coisas (mecanismos de mídia, plugins de formatos de compilação, arquivos de pacote, de tema, de layout e de formato de roteiro, os arquivos dentro de uma pasta de projeto) e as variáveis de ambiente que o programa lê.

O {{APP}} é um único arquivo de programa (o AppImage). Atualizações e extensões vêm como pequenos arquivos de pacote terminados em **.qbpkg**. Há três tipos, e todos são instalados do mesmo jeito:

- **Atualização do programa**: uma versão mais nova do próprio {{APP}}.
- **Complemento**: uma extensão que você pode ativar e desativar, como temas de cores, modelos de projeto, formatos de compilação, dicionários, ferramentas de escrita (seção 10), a reprodução de áudio e vídeo e seus formatos extras (seção 6), importadores de arquivos antigos do Word e de páginas da web (seção 14).

Atualizações do programa e complementos estão na página **Releases** da página do projeto (seção 1), ao lado do AppImage.
- **Pacote de atualização**: um arquivo que contém uma atualização do programa e vários complementos, instalados juntos.

### Verificando atualizações

Desde a 0.92.0, o {{APP}} pode procurar atualizações por conta própria. É a única coisa para a qual o programa entra em contato com a internet por si só, e você pode desativá-la.

- **Ajuda > Complementos e atualizações > Verificar atualizações ao iniciar** é a chave (ativada de início). **Ajuda > Complementos e atualizações > Verificar atualizações agora** olha na hora, seja qual for a chave.
- **A primeira inicialização** avisa isso em sua página, antes de qualquer contato: uma caixa de seleção *Procurar atualizações ao iniciar o programa* (marcada) com a nota de que nada sobre você é enviado; um **OK** encerra a página. Um programa que foi iniciado antes da 0.94.0 e não viu a nota a mostra como uma caixa com **OK** e **Desativar**.
- **O que acontece**: alguns segundos depois da inicialização, no máximo uma vez por dia, o programa obtém um pequeno arquivo da página do projeto (*shelin-boop.github.io/Quillbinder_Collection/updates-1.json*) e a assinatura desse arquivo. A requisição nomeia o programa ("{{APP}}") e nada mais: nem versão, nem sistema, nem identificador, nem cookie. Como em toda conexão, o servidor (o GitHub) vê seu endereço de internet e a hora. O arquivo lista a versão mais nova do programa e dos complementos do projeto; seu programa compara isso com o que está instalado, no seu computador. O arquivo só vale quando traz a assinatura da chave de release do projeto, e um arquivo mais antigo que um já visto é ignorado.
- **Quando há uma atualização**, uma janela a nomeia: o programa ou o complemento, sua versão e a nova, o que há de novo e seu tipo. Uma atualização **funcional** tem três botões: **Baixar e instalar**, **Ignorar esta versão** (sem mais lembretes para essa versão) e **Adiar**. Adiar espera mais a cada vez: 1 dia, 1 semana, 2 semanas, 4 semanas; depois há um último lembrete, e depois dele os lembretes para essa versão terminam. Uma atualização **recomendada (de segurança)** tem apenas dois botões, **Baixar e instalar** e **Adiar**: ela não pode ser ignorada, e depois do quarto adiamento volta a cada 4 semanas. Quando os dois tipos estão presentes, uma janela os lista e segue as regras de segurança. A janela aparece ao lado do seu trabalho e não toma o teclado.
- **Baixar e instalar** obtém os arquivos que a lista nomeia, confere cada um com o tamanho e a soma de verificação da lista assinada e o instala: um complemento ou uma atualização do programa, como faz **Ajuda > Complementos e atualizações > Instalar atualização ou complemento** (apenas pacotes com a assinatura do projeto). Depois, oferece iniciar o programa de novo.
- **Quando a nova versão precisa de um novo AppImage** (outras bibliotecas além das que seu AppImage tem), o {{APP}} o baixa, coloca-o no lugar do AppImage que você iniciou, onde quer que esteja e com o mesmo nome de arquivo, guarda o que você tinha ao lado como *…AppImage.before-update* e inicia de novo. Exclua o arquivo guardado quando a nova versão funcionar para você. Se a pasta do seu AppImage não puder ser gravada, o novo arquivo vai para a sua pasta Downloads e uma mensagem diz o que fazer.
- **No Windows** (0.92.0), essa versão vem como seu programa de instalação. O {{APP}} o baixa para sua própria pasta (*data\update-setup*), confere-o com a lista assinada do projeto e pergunta. Com **Fechar e atualizar agora**, o {{APP}} fecha, e o programa de instalação descompacta a nova versão na pasta que você usa, sem suas páginas, e a inicia; suas configurações, complementos e todo o resto em *data* permanecem. Com **Mais tarde**, nada muda e a atualização é oferecida de novo. Se o Windows não inicia o programa de instalação (ele não leva certificado de assinatura de código), uma mensagem nomeia o arquivo, e você também pode baixar a versão atual da página do projeto.
- **Quando o endereço não pode ser alcançado**, o programa tenta duas vezes, com 5 segundos de intervalo, e depois mostra um pequeno texto na barra de rodapé do primeiro editor, no lugar do texto de salvamento, que aparece e some duas vezes. Nenhuma janela e, no máximo, uma vez por dia. (**Verificar atualizações agora** responde com uma mensagem.)
- **Desativada**, o programa não contata nada, como antes da 0.92.0. Para computadores configurados para outras pessoas, a variável de ambiente `QB_NO_UPDATE_CHECK=1` impede que a verificação automática rode, seja qual for a chave.
- Um AppImage de uma versão anterior à 0.92.0 que executa uma atualização de programa mais nova não consegue fazer conexões seguras; nele, a verificação avisa isso quando você a pede e fica calada nos outros casos. Baixe o AppImage atual uma vez.
- O que o programa lembra sobre isso (a chave, a hora da última verificação, o que você ignorou ou adiou) está em *updates-state.json* na pasta de dados do programa.

### Instalando um pacote

1. Escolha **Ajuda > Complementos e atualizações > Instalar atualização ou complemento…** e selecione o arquivo .qbpkg, ou clique duas vezes no arquivo no seu gerenciador de arquivos.
2. O {{APP}} mostra o que o pacote contém e o que há de novo. Clique em **Instalar**.
3. Se o pacote contém uma atualização do programa, o {{APP}} pede para reiniciar. Seu projeto aberto é salvo e reaberto.

O {{APP}} confere cada pacote antes de instalá-lo: arquivos danificados e pacotes feitos para uma versão diferente são recusados. Uma atualização do programa é iniciada uma vez, de forma invisível, como teste; se ela não roda no seu computador, não é instalada. As partes de um pacote de atualização são instaladas juntas: quando uma delas não pode ser instalada, nenhuma é (0.65).

**Assinaturas** (0.65). Todo pacote na página de releases do projeto é assinado com a chave do projeto, e o {{APP}} leva a metade pública dessa chave. A janela de instalação diz **Assinado pelo projeto Quillbinder** para tal pacote: é o pacote que o projeto publicou, inalterado.

- Um pacote cuja assinatura não confere (alterado depois de assinado, ou danificado) nunca é instalado.
- Uma **atualização do programa** só é instalada com a assinatura do projeto.
- Um **complemento** sem ela (um que você ou outra pessoa fez) só é instalado depois de um aviso claro, com **Cancelar** como padrão. Na linha de comando (`--install <file>`), um pacote sem a assinatura do projeto só é instalado quando você acrescenta `--allow-unsigned`.

**Chaves de complemento** (0.91.3). Complementos oficiais, também os escritos por outros autores, podem ser assinados com uma chave própria em vez da chave do projeto. O projeto atesta tal chave com a sua própria chave, em um certificado que vem dentro do pacote e permite que a chave assine complementos e nada mais. A janela de instalação então nomeia a chave: *Assinado com a chave de complemento "Quillbinder official add-ons", que o projeto Quillbinder atesta*.

- Uma **atualização do programa** assinada com uma chave de complemento é recusada como uma sem assinatura: atualizações do programa precisam da chave própria do projeto.
- Uma chave de complemento que o projeto não atesta conta como sem assinatura: o complemento só é instalado depois do aviso.
- Complementos assinados com a chave própria do projeto, como todos foram até a 0.91.2, continuam confiáveis. A partir da 0.91.3, os complementos próprios do projeto também são assinados com a chave de complemento.
- As versões até a 0.91.2 não conhecem chaves de complemento e avisam antes de instalar um complemento assinado com uma: instale primeiro a atualização do programa (ela é assinada com a chave própria do projeto) e depois os complementos.

**A janela que pergunta antes de um complemento ser instalado** (0.94.0) diz primeiro, em uma ou duas frases, o que o complemento faz. As linhas técnicas (as bibliotecas que ele traz, o mecanismo de fala ou de gráficos, o runtime para o qual foi construído) ficam em **Detalhes**, que está recolhido até você abri-lo.

**Verificando você mesmo um download.** Cada release também contém *SHA256SUMS.txt* (a soma de verificação de cada arquivo), sua assinatura *SHA256SUMS.txt.sig* e a chave pública *quillbinder-signing-public.pem*. Em um terminal, na pasta com os downloads:

    openssl pkeyutl -verify -pubin -inkey quillbinder-signing-public.pem -rawin -in SHA256SUMS.txt -sigfile SHA256SUMS.txt.sig
    sha256sum -c --ignore-missing SHA256SUMS.txt

O primeiro comando responde *Signature Verified Successfully*, o segundo *OK* para cada arquivo que você baixou. Para ter certeza de que o próprio arquivo da chave é o do projeto, compare seus 32 bytes (`openssl pkey -pubin -in quillbinder-signing-public.pem -outform DER | tail -c 32 | od -An -tx1`) com a chave no código-fonte do programa (*src/core/Signing.cpp* e *packaging/quillbinder-signing-public.pem*): eles começam com *bc 3c 58 00 08 3f 66 e6*.

### O Gerenciador de complementos

**Ajuda > Complementos e atualizações > Gerenciador de complementos…** mostra o que está instalado.

- **Versão do programa**: a versão embutida no seu AppImage e cada atualização instalada. A marcada como *em execução agora* é a que está em uso. Para voltar a uma versão anterior, selecione-a e clique em **Usar esta versão**, depois reinicie. Atualizações instaladas de que você não precisa mais podem ser removidas.
- **Complementos**: desmarque um complemento para desativá-lo sem removê-lo; **Remover complemento** o exclui. Temas, modelos e formatos de compilação de um complemento aparecem nos lugares de costume: **Visualizar > Temas**, a janela **Novo projeto** e a janela **Compilar**.
- Quando uma escolha feita aqui não pode ser gravada (disco cheio, pasta que não pode ser gravada), uma mensagem avisa, com o nome do arquivo (0.92.0). Antes, a janela mostrava a escolha como feita e ela era desfeita na inicialização seguinte sem uma palavra. O mesmo vale para uma instalação: um complemento ou uma versão do programa cuja entrada não pôde ser gravada é informado como não instalado, com o que fazer. Quando uma instalação falha no meio e a versão instalada antes não pode ser recolocada, a mensagem nomeia a pasta em que ela está e o nome a dar a essa pasta.

### Se uma atualização não inicia

Se uma atualização do programa falha ao iniciar duas vezes seguidas, o {{APP}} a desativa por conta própria e inicia a versão embutida no AppImage, com uma mensagem avisando isso. Nada nos seus projetos é afetado por isso.

### Quando um novo AppImage é necessário

As atualizações do programa rodam nas bibliotecas que estão dentro do seu AppImage. De tempos em tempos, uma atualização maior precisa de um novo AppImage; a janela de instalação avisa quando um pacote não serve para o seu. Substitua o arquivo AppImage antigo pelo novo; suas configurações, complementos e projetos permanecem como estão.

A 0.80.0 é uma dessas versões: seu AppImage traz um novo Qt (a build própria da The Qt Company do Qt 6.12, em vez do Qt 6.4.2 do Ubuntu). Complementos com código de programa (*Media Playback*, *Extra Video and Audio Formats*, *Speech Transcription*, as ferramentas de escrita e os importadores) precisam ser instalados de novo em suas versões para a 0.80.0 (o pacote de atualização e o **media-all-formats.qbpkg** na página Releases trazem todos); até então, o Gerenciador de complementos marca os antigos como feitos para um programa mais antigo, e seu código não é carregado. A versão completa da 0.80.0 traz todos os complementos.

A 0.93.0 é outra (Linux): seu AppImage traz o Qt 6.12.1, cujas bibliotecas embutidas (zlib, HarfBuzz, PCRE2, libpng, libjpeg-turbo, FreeType e outras) são mais novas e têm correções de segurança. Uma atualização do programa não consegue trazê-las, de modo que a verificação de atualizações oferece o novo AppImage. Os complementos com código de programa do AppImage anterior (os listados acima) são instalados de novo em suas versões para a 0.93.0 da mesma forma, e os complementos de mídia agora trazem o FFmpeg 9.1.

Um novo AppImage também traz uma nova versão do programa por conta própria (por exemplo o AppImage 0.20.0). Ele inicia essa versão, mesmo que você tenha instalado atualizações no AppImage antigo antes: as atualizações mais antigas são desativadas e permanecem listadas em **Ajuda > Complementos e atualizações > Gerenciador de complementos…**, onde você pode removê-las. Se o novo arquivo tem outro nome ou está em outra pasta que o antigo, escolha **Ajuda > Adicionar ao menu de aplicativos** uma vez, para que a entrada do menu e os projetos abertos com clique duplo usem o novo arquivo.

---

## 18. O dialeto Markdown

Você não precisa saber isto para usar o {{APP}}; o editor mostra texto formatado. Só importa se você abrir os arquivos de texto com outro programa. O {{APP}} grava Markdown com estas regras:

- **Cada linha é um parágrafo**, como em um processador de texto. Uma linha vazia é um parágrafo vazio.
- Títulos: `# ` a `###### ` no início da linha.
- Citação em bloco: `> `. Parágrafo centralizado: `->text<-`. Qualquer outro estilo de parágrafo: `::Style Name:: text`.
- Estilos de caractere (a partir da versão 0.4): `<span class="Style Name">text</span>`. Só os nomes dos estilos são gravados no texto. A aparência de um estilo é guardada uma única vez, no arquivo do projeto, de modo que um estilo alterado não exige nenhuma mudança nos documentos.
- Listas: `- ` e `1. `; dois espaços por nível de aninhamento.
- `**bold**`, `*italic*`, `~~strikethrough~~`, `==highlight==`, `<u>underline</u>`, `<sup>superscript</sup>`, `<sub>subscript</sub>`.
- Cores de revisão: `<rev1>…</rev1>` a `<rev5>…</rev5>`.
- Links: `[text](https://example.com)`; links para documentos do projeto: `[text](qb://ID)`.
- Imagens: `![](images/picture.png){width=300}` (as imagens são guardadas na pasta do documento). A descrição de uma imagem (0.62) é o texto entre os colchetes: `![A brass lamp](images/lamp.png){width=300}`.
- Tabelas (0.62): uma linha `:::table` com o design entre chaves (JSON), uma linha por fileira com as células entre `|` (os parágrafos de uma célula separados por `<p/>`, uma célula coberta por uma célula mesclada deixada vazia, um `|` no texto escrito `\|`) e uma linha `:::` no fim.
- Uma imagem ao lado do texto ou centralizada (0.62): `:::figure {"pos":1,"width":168}` (1 à esquerda, 2 à direita, 3 centralizada), a linha da imagem, as linhas da legenda e `:::`.
- Uma quebra de linha dentro de um parágrafo: `<br>`.
- Fontes definidas manualmente (a partir da versão 0.3): `<span style="font-family:'DejaVu Sans'; font-size:14pt; color:#aa0000">text</span>`. As declarações são CSS comum, de modo que outros programas de Markdown que permitem HTML também as mostram. Entradas possíveis: `font-family`, `font-size` (pt), `font-weight` (100 a 900), `color`, `letter-spacing` (em), `font-variant:small-caps`, `text-transform:uppercase`.
- Formatação de parágrafo definida manualmente (a partir da versão 0.3): uma lista entre chaves no início da linha, antes de qualquer outro prefixo: `{: align=right left=10mm right=5mm first=0mm before=6pt after=12pt line=1.5 tabs=20mm,50mmR} text`. `line` é um múltiplo (`1.5`), uma altura exata (`14pt`) ou um mínimo (`14pt+`). As paradas de tabulação terminam com `R` (direita), `C` (centralizada) ou `D` (ponto decimal); nenhuma letra significa esquerda. Só as entradas que você definiu são gravadas.
- Elementos de roteiro (a partir da versão 0.10) são estilos de parágrafo com o nome do elemento: `::Script > Scene Heading:: INT. LIGHTHOUSE - NIGHT`. A aparência de um elemento é guardada uma única vez, no arquivo do projeto, com o formato de roteiro.
- Uma quebra de página antes de um parágrafo (de roteiros e arquivos RTF importados, a partir da versão 0.10): `{: page=before} text`.
- Versões anteriores à 0.62 mostram tabelas e figuras como linhas de texto (`:::table` ...). Se você voltar a uma versão mais antiga, não edite documentos com tabelas ou imagens ao lado do texto.
- A versão 0.2 mostra os dois marcadores como texto simples. Se você voltar à versão 0.2 com o Gerenciador de complementos, não edite documentos que os contenham.
- Versões anteriores à 0.4 não conhecem seus próprios estilos: mostram `<span class=…>` como texto simples e tratam parágrafos de seus próprios estilos como texto normal. Se você voltar a uma versão mais antiga, não edite documentos que usam seus próprios estilos.
- Versões anteriores à 0.10 tratam elementos de roteiro como texto normal e removem os nomes dos elementos de um roteiro assim que ele foi mostrado no editor e o projeto é salvo. Se você voltar a uma versão mais antiga, não abra seus roteiros ali (veja a seção 20).
- Uma barra invertida faz o caractere seguinte virar texto comum: `\*` é uma estrela, não itálico.
- As notas seguem a convenção CriticMarkup:
  - `{>>inline annotation<<}`
  - `^[inline footnote]`
  - `{==text==}{>>linked comment<<}`
  - `{==text==}^[linked footnote]`
  - Um comentário pode começar com uma cor: `{>>@#ff8800 text<<}`.
**Arquivo > Converter arquivo Markdown ⇄ RTF…** lê tanto este dialeto quanto o Markdown padrão (em que os parágrafos são separados por linhas vazias) e, ao gravar Markdown, pergunta qual dos dois você quer.

---

## 19. Atalhos de teclado

### Alterando atalhos

**Arquivo > Opções > Teclado** (ou **Ajuda > Atalhos de teclado**) lista todos os comandos, agrupados por menu, com seu atalho e o atalho padrão com que o programa veio. Os estilos do projeto aberto e os layouts também são listados, de modo que um estilo ou um layout pode ter um atalho próprio.

1. Encontre o comando: role a lista, ou digite uma parte do nome dele ou um atalho como "Ctrl+K" no campo de busca.
2. Clique no comando, clique no campo de atalho abaixo da lista e pressione as novas teclas.
3. Se outro comando tem esse atalho, o programa pergunta se deve tirá-lo de lá. O outro comando fica então sem atalho até você lhe dar um novo.

**Sem atalho** remove o atalho do comando selecionado, **Padrão** devolve o atalho com que ele veio e **Tudo para o padrão** desfaz todas as suas alterações. Suas alterações são mostradas em negrito. Um atalho precisa de Ctrl, Alt ou Meta, ou ser uma tecla de função, porque as letras simples são necessárias para escrever. **Exportar** e **Importar** salvam seu conjunto de atalhos como um arquivo (*.qbkeys*) e carregam um, por exemplo para levá-lo a outro computador.

Os atalhos de estilos pertencem ao nome do estilo: "Citação em bloco" tem seu atalho em todo projeto que tem um estilo com esse nome.

### Atalhos e o layout do teclado

Alguns dos atalhos padrão são teclas que precisam de **AltGr** em um teclado alemão: **Ctrl+{**, **Ctrl+|** e **Ctrl+}** (alinhar à esquerda, centralizar, alinhar à direita), **Ctrl+[** e **Ctrl+]** (voltar e avançar), **Ctrl+\\** (a lista de elementos de roteiro) e **Ctrl+=** (subscrito). Desde a 0.94.0, o programa examina o layout do teclado do sistema **na primeiríssima inicialização** (quando não existem configurações de atalhos nem inicialização anterior) e guarda a resposta. Com um **layout americano**, ou qualquer layout que não seja o alemão, o conjunto usual acima permanece. Com um **layout alemão (ou austríaco)**, esses comandos recebem teclas que não precisam de AltGr (o mesmo conjunto vale com um **layout francês, belga, espanhol, italiano, português, turco ou holandês**, cujos colchetes e chaves também precisam de AltGr; o layout de programador polonês, cujas letras ć e ń do AltGr colidem com Ctrl+Alt+C e Ctrl+Alt+N, move apenas os quatro comandos citados abaixo da tabela):

| Comando | Conjunto alemão |
|---------|------------|
| Alinhar à esquerda / centralizar / alinhar à direita (também no mapa mental) | Alt+Shift+L / Alt+Shift+E / Alt+Shift+R |
| Justificar (mapa mental) | Alt+Shift+J |
| Voltar / avançar | Alt+Shift+B / Alt+Shift+F |
| Lista de elementos de roteiro | Ctrl+Shift+O |
| Subscrito | Ctrl+Shift+D |
| Trazer para a frente / enviar para trás (mapa mental) | Ctrl+Shift+PageUp / Ctrl+Shift+PageDown |

Os layouts que usam o AltGr para letras (polonês, brasileiro e os outros acima) também tiram quatro comandos de Ctrl+Alt+letra, que o Windows lê como AltGr: copiar formatação Ctrl+Shift+Y, colar formatação Ctrl+Shift+U, ir para as notas do documento Ctrl+Shift+X e (não no layout polonês) mostrar régua Ctrl+Shift+W. Teclados russos e indonésios mantêm o conjunto usual.

Os menus mostram cada tecla como o layout a nomeia. Um programa que foi usado antes da 0.94.0 mantém seus atalhos como eram (nada muda por conta própria), e o mesmo vale para todo atalho que você definiu em **Arquivo > Opções > Teclado**: um atalho guardado nunca é substituído. Desde 0.95, o botão **Usar o conjunto do meu layout de teclado…** na parte de baixo de **Arquivo > Opções > Teclado** aplica o conjunto que combina com o layout de teclado em uso agora (para um teclado alemão, as teclas acima). Ele primeiro lista quais atalhos padrão mudam; os atalhos que você definiu permanecem. O conjunto pertence ao seu perfil. Onde o sistema não informa seu layout, usa-se o conjunto usual. (Linux: as configurações de teclado da área de trabalho ou do servidor X; Windows: o idioma do layout do teclado; Android: o idioma do teclado.)

### Os atalhos padrão

| Ação | Atalho |
|--------|----------|
| Novo projeto / abrir projeto | Ctrl+Shift+N / Ctrl+O |
| Salvar / fechar projeto / sair | Ctrl+S / Ctrl+W / Ctrl+Q |
| Novo documento / nova pasta | Ctrl+N / Alt+Shift+N |
| Renomear / duplicar | F2 / Ctrl+D |
| Mover item para cima / para baixo / para fora / para dentro | Ctrl+Alt+Up / Down / Left / Right |
| Dividir na seleção / com a seleção como título | Ctrl+K / Ctrl+Shift+K |
| Mesclar documentos | Ctrl+Shift+M |
| Texto combinado / quadro de cortiça / estrutura de tópicos | Ctrl+1 / Ctrl+2 / Ctrl+3 |
| Mostrar ou ocultar Fichário / inspetor | Ctrl+Alt+B / Ctrl+Alt+I |
| Modo de composição / tela cheia | F11 / Ctrl+Shift+F11 |
| Foco no Fichário / foco no editor / outro editor | Alt+1 / Alt+2 / F6 |
| Voltar / avançar | Ctrl+[ / Ctrl+] (layout alemão: Alt+Shift+B / Alt+Shift+F) |
| Documento anterior / próximo | Alt+Shift+Up / Alt+Shift+Down |
| Mostrar no Fichário | Ctrl+Shift+R |
| Localizar / substituir / próximo / anterior | Ctrl+F / Ctrl+H / F3 / Shift+F3 |
| Busca no projeto | Ctrl+Shift+F |
| Usar seleção para localizar | Ctrl+E |
| Localizar por formatação / próxima / anterior | Ctrl+F3 / Ctrl+G / Ctrl+Shift+G |
| Bloco de rascunho (complemento) | Alt+Shift+Return; de qualquer programa: veja a seção 10 |
| Áudio e vídeo (complemento): reproduzir / pausar | Ctrl+Return |
| Áudio e vídeo: voltar / avançar | Ctrl+Alt+, / Ctrl+Alt+. |
| Áudio e vídeo: inserir marca de tempo | Ctrl+Shift+Return |
| Filtrar quadro de cortiça ou estrutura de tópicos | Ctrl+F (enquanto um deles está com o teclado) |
| Link para documento | Ctrl+L |
| Comentário / nota de rodapé | Shift+F2 / Ctrl+Shift+F2 |
| Anotação no texto / nota de rodapé no texto | Ctrl+Shift+A / Ctrl+Alt+F |
| Fazer instantâneo / com título | Ctrl+5 / Ctrl+Shift+5 |
| Metas do projeto / palavras-chave do projeto | Ctrl+Shift+T / Ctrl+Alt+K |
| Importar arquivos / compilar | Ctrl+Shift+I / Ctrl+Shift+E |
| Imprimir documento atual | Ctrl+P |
| Janela de fontes / janela de parágrafo | Ctrl+T / Ctrl+Alt+P |
| Fonte maior / menor | Ctrl+> / Ctrl+< |
| Copiar formatação / colar formatação | Ctrl+Alt+C / Ctrl+Alt+V |
| Colar com formatação / colar e corresponder ao estilo | Ctrl+Alt+Shift+V / Ctrl+Shift+V |
| Sem estilo / Título 1 / Título 2 / Título 3 | Ctrl+Alt+0 / Ctrl+Alt+1 / Ctrl+Alt+2 / Ctrl+Alt+3 |
| Os próximos seis estilos da lista de estilos (Formatar > Estilo > Personalizar lista de estilos) | Ctrl+Alt+4 ... Ctrl+Alt+9 |
| Ir para a sinopse / ir para as notas do documento (Navegar > Inspecionar) | Ctrl+Alt+Y / Ctrl+Alt+N |
| Fazer backup agora | Ctrl+Shift+B |
| Modo de roteiro / lista de elementos de roteiro | Ctrl+8 / Ctrl+\\ (layout alemão: Ctrl+8 / Ctrl+Shift+O) |
| Em um roteiro: próximo elemento / alterar elemento | Enter / Tab (veja a seção 4, Roteirização) |
| Aumentar zoom / reduzir zoom / redefinir zoom | Ctrl++ / Ctrl+- / Ctrl+0 |
| Zoom da área sob o mouse | Ctrl+roda do mouse |
| Mostrar ou ocultar a régua | Ctrl+Alt+R |
| Rolagem de máquina de escrever | Meta+Ctrl+T |
| Opções | Ctrl+, |
| Este manual | F1 |

---

## 20. Solução de problemas

### Relatórios de falhas e informações do sistema

Se o {{APP}} termina por causa de um erro, ele grava um **relatório de falha** no seu computador antes de fechar. Se a janela dele não responde por 30 segundos (ela trava; o tempo é definido em **Arquivo > Opções > Extras > Relatórios de falhas**), ele grava um **relatório de travamento** que mostra onde ele está preso; se ele se recupera, o relatório diz depois de quanto tempo. Na próxima vez que você iniciar o {{APP}} depois de uma falha, ou depois de ter precisado encerrar um {{APP}} travado, uma mensagem diz o que aconteceu. Se ele terminou em até um minuto depois de iniciar, a mensagem oferece **Iniciar sem o último projeto**, caso o projeto seja a causa.

- **Ajuda > Solução de problemas e desenvolvimento > Relatórios de falhas…** lista os relatórios de falha e de travamento com sua data e o tipo de erro ("travamento (recuperado)" quando o {{APP}} continuou funcionando). **Copiar para a área de transferência** copia o selecionado, para que você possa colá-lo em uma mensagem para quem cuida do {{APP}} para você; **Abrir pasta** mostra os arquivos; **Excluir** e **Excluir tudo…** os removem. Os 20 relatórios mais novos são mantidos (**Arquivo > Opções > Extras > Relatórios de falhas** altera o número).
- **Arquivo > Opções > Geral > Avisos > Mostrar console de registro interno** mostra as mensagens do programa e do Qt desde que ele foi iniciado (as últimas 1000), com as novas à medida que chegam; útil quando algo não funciona e nenhuma mensagem diz por quê.
- **Ajuda > Solução de problemas e desenvolvimento > Informações do sistema…** mostra o mesmo retrato do programa e do computador que um relatório carrega. Ele começa com **Pontos de atenção**: o que pode causar problemas em breve, como uma unidade quase cheia ou somente leitura, um projeto em uma pasta de rede, uma unidade de backup com menos espaço livre do que o projeto precisa, pouca memória livre, um tempo longo de salvamento automático, backups automáticos desativados ou relatórios recentes de falhas e travamentos. Depois vêm a versão do programa, o runtime (o conjunto de bibliotecas dentro do AppImage, com o dia de lançamento de seu Qt e a idade dele: um runtime antigo significa que um AppImage mais novo pode trazer correções de segurança) e as atualizações e os complementos instalados, o sistema operacional, a área de trabalho, as telas (desde a 0.93.1 com a área que uma janela pode usar em cada uma), os dispositivos de entrada (teclado, mouse, tela sensível ao toque, caneta) e se o teclado na tela é mostrado (0.93.1), o processador e a memória, as pastas com seu espaço livre, as configurações de salvamento e de backup e o projeto aberto. Use **Copiar para a área de transferência** ou **Salvar como…** quando relatar um problema que não é uma falha.

Um relatório nomeia arquivos, pastas e documentos, além dos comandos e das teclas usados pouco antes do erro (as letras digitadas são apenas contadas, nunca registradas). Ele não contém texto dos seus documentos. Sua pasta pessoal é escrita como **~**.

Se o {{APP}} foi encerrado de fora ou o computador foi desligado sem fechá-lo, não há relatório, mas a próxima inicialização também avisa. Nos dois casos, seu trabalho está salvo até o último salvamento automático, alguns segundos antes do fim; olhe os instantâneos do documento ou o backup mais novo se algo estiver faltando (seções 11 e 15).

**"O arquivo do projeto não pode ser lido."** Se o arquivo do projeto (*SeuTítulo*{{FILE}}) está danificado, por exemplo por uma falha do computador enquanto era salvo, o {{APP}} oferece a cópia que guarda dele (da última vez em que o projeto abriu): **Usar a cópia**. O arquivo danificado é guardado ao lado dela (*… .damaged-<data>*). As alterações no Fichário desde então (títulos, ordem, etiquetas) estão ausentes; os textos estão todos lá, e os documentos criados desde então são colocados na pasta **Documentos recuperados** em Pesquisa, com o nome de sua primeira linha.

**"O arquivo do projeto continha caracteres que não são permitidos nele."** Versões anteriores à 0.20 podiam salvar caracteres de controle invisíveis (colados de outros programas em um título ou em uma sinopse), depois do que o projeto deixava de abrir. O {{APP}} os remove quando abre o projeto e avisa; nada mais é alterado. Desde a 0.20, eles nunca são salvos.

**Um documento é mostrado vazio, com a nota "O arquivo deste documento não pode ser lido".** O arquivo de texto dele existe, mas não pode ser lido (suas permissões foram alteradas, ou a unidade em que ele está sumiu). O {{APP}} o mostra vazio e não deixa você editá-lo, para que o arquivo nunca seja sobrescrito. Verifique as permissões do arquivo ou a unidade e depois feche e abra o projeto de novo.

**Pressionar Page Down ou Page Up fechou o programa.** Isso era um erro em versões anteriores à 0.19; atualize para a 0.19 ou posterior. Até lá, role com o mouse ou com a barra de rolagem.

**O programa não inicia quando clico duas vezes nele.** Clique com o botão direito no arquivo > Propriedades > Permissões e marque "É executável" (no Dolphin: "Permitir executar o arquivo como programa"). Depois clique duas vezes nele de novo.

**As caixas de diálogo de arquivos têm aparência diferente da de outros programas do KDE.** Ative **Usar as caixas de diálogo de arquivos da área de trabalho** em **Arquivo > Opções > Geral** e reinicie o {{APP}}.

**Os ícones da barra de ferramentas são difíceis de ver em um tema escuro ou claro.** O {{APP}} escolhe o conjunto claro ou escuro dos ícones da sua área de trabalho para combinar com o tema. Se a área de trabalho não tem um conjunto adequado, ele usa seus próprios ícones. Se ícones isolados ainda não combinam, escolha **Padrão (seguir a área de trabalho)** em **Visualizar > Temas** e defina as cores nas configurações da própria área de trabalho.

**Alterei cores e quero as do tema de volta.** **Arquivo > Opções > Aparência > Redefinir cores e fontes para o tema**, ou o botão **↺** ao lado de uma única cor ou fonte.

**Uma atualização do programa não iniciou.** O {{APP}} desativa por conta própria uma atualização que falha (seção 17). Você pode tentar instalá-la de novo, ou continuar usando a versão embutida.

**Falta um formato de compilação.** Verifique **Arquivo > Opções > Plugins**: ali consta de qual programa o formato precisa. Instale-o no Discover e reinicie o {{APP}}.

**O corretor ortográfico não conhece meu idioma.** Para espanhol, francês, português do Brasil, italiano, russo, indonésio, turco, polonês e holandês, instale o complemento "dicionário ortográfico de <idioma>" (**Ajuda > Complementos e atualizações > Instalar atualização ou complemento**; a página de download os lista), reinicie o {{APP}} e escolha o idioma em **Arquivo > Opções > Correções** (página Ortografia). Para outro idioma, instale o pacote de dicionário *hunspell* correspondente no Discover.

**O projeto "parece estar aberto em outra janela ou em outro computador".** Isso acontece quando o projeto está em uma pasta sincronizada que está aberta em outro lugar, ou depois de uma falha em outro computador. Se você tem certeza de que ele não está aberto em nenhum outro lugar, escolha **Sim**.

**Alt+arrastar não abre um suporte de cópia.** Algumas áreas de trabalho usam Alt+arrastar para algo próprio, como mover janelas. Use **Navegar > Abrir > no suporte de cópia** ou o menu de clique com o botão direito do documento, ou altere a tecla nas configurações de janelas da área de trabalho.

**Depois de voltar a uma versão mais antiga do programa, um roteiro é texto simples, sem seu layout.** Versões anteriores à 0.10 não conhecem elementos de roteiro e salvam como texto comum um roteiro que mostraram. Volte à versão atual (**Ajuda > Complementos e atualizações > Gerenciador de complementos**). O texto está todo lá; para ter o layout de volta, restaure o documento a partir de um instantâneo ou de um backup feito antes (seções 11 e 15), ou dê de novo aos parágrafos seus elementos com o menu pop-up no rodapé.

**O atalho do bloco de rascunho não faz nada em outros programas.** Atalhos definidos em **Arquivo > Opções > Teclado** só funcionam dentro do {{APP}}. Para outros programas, a área de trabalho precisa iniciar o comando (seção 10, *De qualquer programa*). Se você mover o AppImage, digite o comando de novo (ele nomeia o lugar do AppImage).

**A câmera do bloco de rascunho diz que não consegue ler a tela.** No Wayland, apenas o programa de captura de tela da própria área de trabalho pode fazer isso. Instale o *Spectacle* (`sudo apt install kde-spectacle`) e tente de novo.

**Editar > Ferramentas de escrita diz "Nenhuma ferramenta de escrita instalada".** As ferramentas são complementos: instale o pacote da atualização 0.11 ou ative-as em **Ajuda > Complementos e atualizações > Gerenciador de complementos**.

**Um arquivo de áudio ou vídeo só oferece "Abrir com o aplicativo padrão".** Reproduzir dentro do {{APP}} precisa do complemento *Media Playback* (no pacote da atualização 0.12); instale-o ou ative-o em **Ajuda > Complementos e atualizações > Gerenciador de complementos**. A linha abaixo do botão diz o que está faltando.

**Um vídeo toca, mas não há som, ou um arquivo de áudio diz "Nenhuma saída de som foi encontrada".** O {{APP}} reproduz o som pelo servidor de som da área de trabalho (PipeWire ou PulseAudio). Verifique se o som funciona em outros programas e se a saída certa está escolhida nas configurações de áudio da área de trabalho; o {{APP}} acompanha na hora uma mudança de saída.

**"Este arquivo não pode ser reproduzido aqui: ele usa H.264 e AAC ..."** O arquivo está em um formato licenciado por patentes, que o *Media Playback* deixa de fora. Instale o complemento **Extra Video and Audio Formats**, ou o pacote **media-all-formats.qbpkg**, que o traz junto com o *Media Playback* (ambos na página Releases da página do projeto, seção 1), e reinicie o {{APP}}.

**Um arquivo não toca, ou toca sem imagem.** Alguns tipos não fazem parte da decodificação embutida (vídeo AV1, WMA Pro, WMV 9). Abra o arquivo com outro programa (clique com o botão direito no reprodutor: **Abrir com o aplicativo padrão**) ou converta-o para outro formato.

**Um arquivo .doc é adicionado como arquivo, não como texto.** O complemento **Word 97–2003 Import** não está instalado ou está desativado (**Ajuda > Complementos e atualizações > Gerenciador de complementos**); depois de instalá-lo, reinicie o {{APP}}. Uma mensagem após a importação nomeia o que o complemento deixou de fora. Arquivos do Word 6.0 e 95 precisam do LibreOffice (`sudo apt install libreoffice-writer`).

**Uma página da web entra vazia ou com a parte errada.** Desmarque **Somente o artigo** na janela Página da web para pegar a página inteira e exclua o que você não precisa. Páginas que exigem login ou são montadas inteiramente por scripts não podem ser lidas sem um navegador: abra a página no seu navegador, selecione o texto e arraste-o para o Fichário (ele vira um novo documento). "Páginas da web seguras não podem ser obtidas" significa que falta o OpenSSL 3 (`sudo apt install libssl3`, parte de todo Ubuntu) ou que o complemento está danificado (instale-o de novo).

**O primeiro segundo de um arquivo é silencioso ou o tempo fica parado no início.** A saída de som precisa de um momento para iniciar; com alguns servidores de som, mais de um segundo. Volte um pouco (Ctrl+Alt+,) se você perdeu o início.

**As páginas na visualização de página não correspondem ao meu manuscrito impresso.** Não é para corresponderem: a visualização de página mostra seu texto em folhas enquanto você escreve. O formato de compilação decide as páginas do arquivo final (seção 13).

**"O Scrivener está com este projeto aberto."** O Scrivener deixa um arquivo de bloqueio no *.scriv* enquanto está com o projeto aberto, e depois de uma falha. Feche o projeto no Scrivener (em todos os computadores) e abra-o de novo aqui; se o Scrivener não está em execução em lugar nenhum, escolha **Sim**.

**Um projeto no formato do Scrivener não mostra minhas últimas alterações feitas no Scrivener.** O {{APP}} só lê o *.scriv* de novo quando o projeto é aberto. Feche-o aqui (**Arquivo > Fechar projeto**), deixe o Scrivener ou o serviço de sincronização terminar e abra-o de novo.

**Os menus Inserir, Navegar, Projeto, Documentos e Formatar estão acinzentados.** Nenhum projeto está aberto. Abra ou crie um (menu Arquivo).

**Onde relatar um problema.** **Ajuda > Relatar um problema…** abre no seu navegador a página de issues do projeto (https://github.com/Shelin-boop/Quillbinder_Collection/issues/new/choose), com um formulário para um problema e outro para um desejo; você precisa de uma conta do GitHub e pode escrever em inglês ou em alemão. As issues são públicas: não anexe seu manuscrito nem seu projeto; **Ajuda > Solução de problemas e desenvolvimento > Informações do sistema…** (**Copiar para a área de transferência**) contém o que um relatório precisa sobre o programa e o computador. Um **problema de segurança** não é relatado ali, mas em particular, como descrito em *SECURITY.md* do projeto (a aba Security do repositório, **Report a vulnerability**).

**O sumário de um arquivo do Word não tem números de página.** Os números de página são preenchidos pelo processador de texto quando ele atualiza o campo; atualize-o lá.

**Algo deu errado com um documento.** Veja a aba de instantâneos do inspetor ou restaure um backup (seção 15).

## Créditos e licença

- Desenvolvedor principal: Claude.AI w. Opus 5.5 Extra

O {{APP}} é software livre sob a Apache License 2.0: qualquer pessoa pode usá-lo, copiá-lo, alterá-lo e repassá-lo, com ou sem cobrança, mantendo a licença e o arquivo NOTICE, que nomeia o projeto e seu código-fonte; sem garantia. As versões até a 0.70.0 foram publicadas sob a licença Zero-Clause BSD (0BSD). A janela Sobre (no menu Ajuda) mostra a versão e a licença.

O {{APP}} usa partes de outros projetos (Qt, Hunspell, ICU, os ícones Breeze e as outras bibliotecas dentro do AppImage), cada uma sob sua própria licença. **Ajuda > Licenças de terceiros…** (também um botão da janela Sobre) as lista com seus textos de licença completos; os mesmos textos estão dentro do AppImage, em *usr/share/doc/quillbinder/licenses*. O código-fonte de cada uma, exatamente na versão usada, está na release **Third-party source code** da página do projeto (https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/third-party-sources); a janela nomeia o arquivo de cada biblioteca, e as notas de licença dos complementos fazem o mesmo para as deles.

Scrivener e Scapple são nomes de produtos da Literature & Latte. O {{APP}} é um programa independente, não feito pela Literature & Latte nem ligado a ela; ele lê e grava os formatos de arquivo deles para que seus projetos possam circular entre os programas.
