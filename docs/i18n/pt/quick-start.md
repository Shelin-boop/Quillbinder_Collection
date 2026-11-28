# Guia de início rápido do {{APP}}

O {{APP}} é um programa de escrita para textos longos. Se você já trabalhou com outros programas de escrita baseados em fichário, quase tudo vai parecer familiar: um Fichário à esquerda, o editor no meio, um inspetor à direita e a compilação no fim. Este guia mostra onde estão as configurações importantes e resume o que o {{APP}} sabe fazer. O manual completo fica em **Ajuda > Manual do usuário** (**F1**; ele tem um sumário e uma busca); um passeio curto pelos recursos que você talvez ainda não conheça está em **Ajuda > Projeto de boas-vindas**.

---

## Primeiros passos

- Quando nenhum projeto está aberto, a **janela inicial** oferece **Novo projeto** (com modelos como Em branco, Romance, Conto e Livro de não ficção), **Projetos recentes**, **Abrir outro…** e **Importar projeto do Scrivener…**.
- O {{APP}} salva sozinho alguns segundos depois que você para de digitar. **Ctrl+S** salva na hora.
- Na primeira vez que o {{APP}} inicia, ele abre o **projeto de boas-vindas**, salvo na sua pasta *Documentos*. Exclua-o quando quiser; **Ajuda > Projeto de boas-vindas** cria um novo.

---

## Onde estão as configurações importantes

| O que você quer | Onde |
|-----------------|------|
| **Modo escuro** ou outro tema de cores | **Visualizar > Temas**: Dark Mode, Midnight, Sepia, Slate, Ocean, High Contrast, Light ou Default (segue a sua área de trabalho). A mudança vale na hora. |
| Cores e fontes de cada parte da janela | **Arquivo > Opções > Aparência** (ou **Visualizar > Temas > Cores e fontes…**), ou um clique com o botão direito na parte (**Aparência**). Cada projeto guarda a sua própria aparência; a lista no alto escolhe este projeto ou os novos projetos. |
| Seu próprio tema | **Visualizar > Temas > Salvar aparência atual como tema…**; **Gerenciar temas…** exporta e importa arquivos *.qbtheme*. |
| Fonte da interface, tamanho dos ícones da barra de ferramentas, o caminho completo na barra de título | **Arquivo > Opções > Aparência > Interface geral** |
| A coluna de texto: largura fixa, cor do papel, largura, borda ao lado do texto | **Arquivo > Opções > Aparência > Editor principal**, ou um clique com o botão direito no papel |
| As configurações do projeto em um arquivo, e de volta para este ou outro projeto | **Projeto > Salvar configurações do projeto em arquivo…** / **Carregar configurações do projeto de um arquivo…** |
| Fonte e formatação de parágrafo dos **novos** projetos | **Arquivo > Opções > Edição > Formatação** ("Usar a formatação do editor atual") |
| Fonte e formatação de parágrafo **deste** projeto | **Projeto > Configurações do projeto > Formatação** |
| Zoom | **Ctrl+roda do mouse** sobre qualquer parte da janela: cada parte guarda o seu próprio zoom. Valores iniciais: **Arquivo > Opções > Edição**. |
| Barras de ferramentas | **Visualizar > Barras de ferramentas > Personalizar barras de ferramentas…** (ou clique com o botão direito em uma barra) |
| A barra de ícones | **Visualizar > Barras de ferramentas > Barra de ícones**; **Escolher ícones para a barra de ícones…** |
| Atalhos de teclado | **Arquivo > Opções > Teclado** (busca, pressionar novas teclas, exportar e importar); **Ajuda > Atalhos de teclado…** os lista |
| Encontrar um comando nos menus | **Ajuda > Localizar comando…** (digite parte do nome; Enter o executa) |
| Layouts de janela | **Visualizar > Layout** (sete internos; salve os seus) |
| Salvamento automático, projetos recentes, informações do autor | **Arquivo > Opções > Geral** |
| Backups | **Arquivo > Opções > Backup** (quando, quantos, onde); **Arquivo > Backup** faz um agora |
| Ortografia e correções enquanto você digita | **Arquivo > Opções > Correções** |
| Onde os links abrem, a tecla Enter, o modo de composição, a reprodução | **Arquivo > Opções > Comportamentos** |
| Configurações de importação e exportação | **Arquivo > Opções > Compartilhamento** |
| Mapas mentais, objetos, complementos, relatórios de falhas | **Arquivo > Opções > Extras** |
| Etiquetas, status, tipos de seção, metadados personalizados, pastas especiais | **Projeto > Configurações do projeto** |
| Opções somente para um projeto | **Arquivo > Opções**, **Configurações para: Este projeto** no alto; se as configurações próprias dos projetos têm prioridade: **Arquivo > Opções > Geral** |
| Levar suas configurações para outro computador | **Arquivo > Opções > Gerenciar > Exportar perfil… / Importar perfil…** |
| Tudo de volta ao padrão | **Arquivo > Opções > Padrões** (seus projetos, temas, layouts e atalhos permanecem) |

---

## O que o {{APP}} sabe fazer

### Escrever e organizar

- **Fichário, editor, inspetor**: documentos e pastas de qualquer tamanho, sinopse e notas, etiquetas e status, palavras-chave, metadados personalizados, favoritos, instantâneos, comentários e notas de rodapé (ao lado do texto ou no texto).
- **Três visualizações** de uma pasta: **Texto combinado** (**Ctrl+1**, todos os documentos dela como um texto contínuo que você pode editar), **Quadro de cortiça** (**Ctrl+2**, fichas, livre, fios de etiqueta) e **Estrutura de tópicos** (**Ctrl+3**).
- **Dois editores** lado a lado ou um acima do outro, **suportes de cópia** dentro de um editor, janelas de **Referência rápida**, **modo de composição** (**F11**), visualização de página, a régua, estilos, modo de revisão, roteirização (roteiro de cinema, peça de teatro, quadrinhos), metas, estatísticas e histórico de escrita.
- **Busca**: busca rápida, busca no projeto, **Substituir no projeto** e **Localizar por formatação**; as buscas salvas viram coleções.

### Seus projetos: dois formatos

- Um **projeto do {{APP}}** é uma pasta: cada documento é um arquivo de texto comum, em **Markdown** (o padrão) ou **RTF**, para que a sua escrita continue legível sem o programa.
- **Importar projeto do Scrivener…** (**Arquivo > Importar**) cria um projeto do {{APP}} a partir de um projeto do Scrivener 3 ou 2 e deixa o original intacto.
- **Trabalhar no formato do Scrivener**: abra o arquivo *.scrivx* de um projeto do Scrivener 3 e escolha **Trabalhar no formato do Scrivener**. O {{APP}} passa a salvar na pasta *.scriv*, de modo que o projeto continua legível pelo Scrivener. **Arquivo > Salvar como…** salva um projeto no outro formato (ou com outro nome), e **Arquivo > Exportar** salva uma cópia que deixa o projeto em que você trabalha como está.

### Trazer e levar texto

- **Importar**: Word (*.docx*), LibreOffice (*.odt*), RTF, texto simples, Markdown, roteiros Fountain e Final Draft, imagens, arquivos PDF, áudio e vídeo. **Importar e dividir…** corta um arquivo longo em documentos, também nos títulos dele (.docx, .odt, .md), com uma pré-visualização do Fichário que isso cria. **Documentos > Agrupar** coloca os itens selecionados em uma nova pasta.
- **Compilar** (**Ctrl+Shift+E**): Word, PDF e e-book já incluídos; Fountain e Final Draft para roteiros; formatos LibreOffice, Pandoc e Markdown como plugins. Formatos de compilação, tipos de seção, pré-texto e pós-texto, substituições e uma pré-visualização.
- **Comparar e transferir** (**Arquivo > Comparar e transferir**): quando um manuscrito volta de um editor como arquivo do Word ou do LibreOffice, o {{APP}} mostra o que difere e assume o lado que você escolher, capítulo por capítulo ou alteração por alteração. O projeto recebe instantâneos antes; uma cópia do Word recebe alterações rastreadas.
- **Exportar** documentos como arquivos e **imprimir** textos, fichas e a estrutura de tópicos.

### Além do comum

- **Mapas mentais**: um quadro de notas, linhas e setas para cada documento e pasta (o botão **Mapa mental** do inspetor). **Ajuda > Ajuda dos mapas mentais** mostra como funciona.
- **Objetos**: personagens, locais, ambientes, pontos de interesse, clima e categorias suas, cada um com uma ficha de campos e links entre eles (o botão **Objetos** do inspetor, ou **Janela > Objetos**).
- **Tabelas e imagens**: **Inserir > Tabela…** com designs prontos ou os seus, **Formatar > Tabela** para linhas, colunas, células mescladas e cores; imagens ao lado do texto com o texto fluindo em volta delas, legendas e descrições que os e-books e os arquivos do Word guardam como texto alternativo (clique com o botão direito em uma imagem, **Imagem…**).
- **A barra de ícones**: até dez ícones de documento, um clique para cada. **Documentos > Alterar ícone** também cria ícones a partir de texto ou das suas próprias imagens.
- **Zoom por parte**: Fichário, editores, partes do inspetor, quadro de cortiça, estrutura de tópicos e mapas mentais guardam cada um o seu zoom.
- **Lugares guardados**: cada documento abre onde você deixou o cursor, também depois de reabrir o projeto.
- **Fontes**: **Formatar > Fonte > Instalar arquivos de fonte…** adiciona fontes sem sair do programa.
- **Segurança**: salvamento automático também enquanto você continua digitando, backups, uma cópia reserva do arquivo do projeto, relatórios de falhas e de travamentos (**Ajuda > Solução de problemas e desenvolvimento > Relatórios de falhas…**) e **Ajuda > Solução de problemas e desenvolvimento > Informações do sistema**.
- **Live Sync** (desativado até você ativá-lo em **Arquivo > Opções > Live Sync**): mantém um projeto igual nos seus dispositivos na mesma rede. Pareie os dispositivos uma vez (**Parear um dispositivo...**: compare os seis dígitos nos dois), depois escolha **Compartilhar este projeto...** no menu do botão Live Sync da barra de ferramentas e **Entrar em um projeto...** no outro dispositivo. Um dispositivo edita um documento por vez; os outros podem pedir. Os arquivos de um projeto que não é criptografado, e os backups ZIP dele, não são criptografados pelo Live Sync: use a criptografia dos seus próprios dispositivos (seção 21 do manual).

### Atualizações e complementos

A **versão completa** do AppImage (*...-full-{{LABEL}}-x86_64.AppImage*) traz todos os complementos dentro e os instala na primeira inicialização. Atualizações e extensões vêm como pequenos arquivos de pacote (*.qbpkg*), na página de versões do projeto (https://github.com/Shelin-boop/Quillbinder_Collection/releases), ao lado do AppImage. Instale-os com **Ajuda > Complementos e atualizações > Instalar atualização ou complemento…** (ou clique duas vezes no arquivo). **Ajuda > Complementos e atualizações > Gerenciador de complementos…** ativa e desativa complementos e volta a uma versão anterior do programa se uma atualização não servir para você. Os complementos incluem temas extras, modelos de projeto, formatos de compilação, o Name Generator, o Scratch Pad, o Text Statistics, a reprodução de áudio e vídeo (um pacote para os formatos mais usados, *media-all-formats.qbpkg*; ou *media-playback.qbpkg* só para os formatos livres) e a importação de páginas da web e de arquivos antigos do Word.

---

## Ajuda

- **Ajuda > Manual do usuário** (**F1**): todas as funções, com os caminhos de menu; F1 abre a parte que combina com o que você está olhando, e **Ctrl+F** na janela dele busca no texto.
- **Ajuda > Projeto de boas-vindas**: um projeto curto que mostra os recursos listados em "Além do comum" e deixa você experimentá-los.
- **Ajuda > Ajuda dos mapas mentais**: notas, linhas, setas e formas, com imagens e um quadro para praticar.
- **Ajuda > Lista de todos os espaços reservados** e **Ajuda > Atalhos de teclado…**
- **Ajuda > Licenças de terceiros**: as licenças das partes de outros projetos que o {{APP}} usa.
- **Ajuda > Sobre o {{APP}}**: a versão e a licença. O {{APP}} é software livre sob a Apache License 2.0.
