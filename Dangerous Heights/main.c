#include <SOIL/SOIL.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_VIDAS 4
#define LARGURA_DO_MUNDO 160
#define ALTURA_DO_MUNDO 120

#define LINHAS_FASE 100
#define COLUNAS_FASE 40
#define LINHAS_CENA 20

#define MAX_VIDAS_BOSS 10

/*
    Menu = 0, Controles = 1, fase = 2;
*/
//gcc menu.c main.c SOIL//*.c -o programa -lopengl32 -lglu32 -lfreeglut

int mouse[2];
int teclas[5];

int tela_atual = 0, fase_atual = 1;

int fase[LINHAS_FASE][COLUNAS_FASE];
double sizeWidth = (double)LARGURA_DO_MUNDO / COLUNAS_FASE;
double sizeHeight = (double)ALTURA_DO_MUNDO / LINHAS_CENA;

const int QUANTIDADE_INIMIGOS[] = {12, 21, 6, 16, 13};

Button *buttons[4];
Button btnStart = {LARGURA_DO_MUNDO / 2, 60, 40, 8, {0.6, 0.1, 0.1}, 1, 0};
Button btnControls = {LARGURA_DO_MUNDO / 2, 50, 40, 8, {0.1, 0.6, 0.1}, 2, 0};
Button btnVoltar = {LARGURA_DO_MUNDO / 2, 15, 40, 8, {0.1, 0.1, 0.6}, 10, 0};

int quantidade_botoes = 3;
int quantidade_inimigos = 0;
int quantidade_projeteis = 0;
double gravidade = 0.5;

GLuint carregaTextura(const char* arquivo) {
    GLuint idTextura = SOIL_load_OGL_texture(
                           arquivo,
                           SOIL_LOAD_AUTO,
                           SOIL_CREATE_NEW_ID,
                           SOIL_FLAG_INVERT_Y
                       );

    if (idTextura == 0) {
        printf("Erro do SOIL: '%s' %s\n", SOIL_last_result(), arquivo);
    }

    return idTextura;
}

typedef struct {
    GLuint grama[5];
    GLuint plataforma[5];
    GLuint fundo0[5], fundo1[5], fundo2[5], vida1, vida2, gameover;
    GLuint blocoDeslizante, masmorra;
    GLuint menu, botaoIniciar, botaoControles, botaoVoltar, telaControles;
    GLuint powerUpArmadura, vidasPowerUp, creditos;
}TexturasMundo;

typedef struct {
    GLuint magoVerde, magoRoxo, fantasma, magoMarrom, morcego, esquimo;
    GLuint verdeMorte, roxoMorte, fanstasmaMorte, marromMorte, morcegoMorte, esquimoMorte;
}TexturasInimigos;

typedef struct{
    int x;
    int y;
    double bottom;
    double top;
    double barraVertical;
    double barraHorizontal;
    int travada;
    int arena;
}Camera;

typedef struct{
    int x,y;
    float width, height;
    int id;
    int existe;
}PowerUp;

typedef struct
{
    double x;
    double y;
    double width;
    double height;
    double movX;
    double movY;
    int i;
    int j;
    int maxPulo;
    int pulo;
    int direcao;
    int podeAtacar;
    int podeDefender;
    int estaDefendendo;
    int vidas;
    GLuint textura;
    float spriteX, spriteY;
    int levouDano;
    int travado;
    float rotacao, fatorEscalaX, fatorEscalaY;
    int idPowerUp;
} Personagem;

typedef struct
{
    double x;
    double y;
    double width;
    double height;
    double velX;
    double velY;
    int vidas, morreu;
    int visao;
    int id;
    int timer;
    GLuint textura;
    float spriteX, spriteY;
    int levouDano;
    int direcao;
} Enemy;

typedef struct{
    int id;
    double x;
    double y;
    double width;
    double height;
    double vel_X;
    double vel_Y;
    int dano;
    int existe;
    int timer;
    //Se o projetil rotaciona
    double raio, angulo, xCentro, yCentro;
}Projetil;

typedef struct
{
  //  char Nome; //Nome do Boss (pode aparecer na tela);
    int vida; //Vida atual do boss(pode aparecer como uma barra de vida na tela)
    double x, y; //Posicao do boss
    double width, height; //Medidas do boss
    double velX, velY; //Velocidades do boss
    int existe; //Variavel para criar/destruir o boss;
    int timerMovimento;
    int timerBomba; // timer pra Bomba
    int timerTeleguiado; //timer pro Teleguiado
    int timerAtaque3; //Possivel terceiro ataque
    GLuint textura; //Textura do Boss
    float spriteX, spriteY;
    int levouDano;
    int ativo;
    double angulo;
    int posicao;
} Boss;

typedef struct {
    int fase;
    double xCentro, yCentro, raio, angulo;
    int sentido;
}Obstaculo;

typedef struct{
    int fase, totalInimigos, tipos[4], total_tipos;
    int posicoesX[10], posicoesY[10], total_posicoes;
    int timer, tempoDeRecarga, comecou;
    double alturaMinima;
} Round;

typedef struct{
    int pontuacao_atual;
    int maior_pontuacao;
}Game;

typedef struct{
    double width, height;
    double x,y;
    float spriteX;
    GLuint textura;
    int existe;
}Portal;

Portal portal;
PowerUp powerUps[50];
Game game = {100, 0};

Boss boss = {MAX_VIDAS_BOSS, LARGURA_DO_MUNDO/2, 435, 10, 10, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};


typedef struct{
    int vida; //Vida atual do boss(pode aparecer como uma barra de vida na tela)
    double x, y; //Posicao do boss
    double width, height; //Medidas do boss
    double velX, velY; //Velocidades do boss
    int timer;
    int espiral;
    int timerEspiral;
    int timerAtaque1;
    int timerMovimento;
    int existe; //Variavel para criar/destruir o boss;
    GLuint textura; //Textura do Boss
    float spriteX, spriteY;
    int levouDano;
    int ativo;
    double angulo;
    double raio;
    int travado;
}BossFinal;

BossFinal bossFinal = {MAX_VIDAS_BOSS, LARGURA_DO_MUNDO/2, 520, 20, 20, 1, 1, 0, 0, 5, 5, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0};

Personagem personagem;
Projetil *projeteis = NULL;
Enemy *inimigos = NULL;

TexturasInimigos texturasInimigos;
TexturasMundo texturasMundo;
Camera camera;

Obstaculo obstaculos[5];
int quantidade_obstaculos = 5;

Round rounds[3];
int listas_fases[3];

void escreveTexto(void * fonte, char *texto, float x, float y, float z) {
    int i;
    glPushMatrix();
        glTranslatef(x, y, z);
        glRasterPos3f(0, 0, 0);
        for (i = 0; i < strlen(texto); i++) {
            glutBitmapCharacter(fonte, texto[i]);
        }
    glPopMatrix();
}

void transformarCoordenadas(double *xMundo, double *yMundo, double x, double y)
{
    double razaoX = LARGURA_DO_MUNDO / (double) (glutGet(GLUT_WINDOW_WIDTH) - camera.barraVertical);
    double razaoY = ALTURA_DO_MUNDO / (double) (glutGet(GLUT_WINDOW_HEIGHT) - camera.barraHorizontal);

    *xMundo = (x - camera.barraVertical/2) * razaoX;
    *yMundo = ALTURA_DO_MUNDO - ((y - camera.barraHorizontal/2) * razaoY);

    *yMundo += camera.bottom;
}

void desenharRetangulo(double width, double height)
{
    glBegin(GL_POLYGON);
    glVertex3f(-width / 2, height / 2, 0);
    glVertex3f(width / 2, height / 2, 0);
    glVertex3f(width / 2, -height / 2, 0);
    glVertex3f(-width / 2, -height / 2, 0);
    glEnd();
}

void redimensionaCena(int width, int height)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if(tela_atual != 2)
        glOrtho(0, LARGURA_DO_MUNDO, 0, ALTURA_DO_MUNDO, -1, 1);
    else{
        glOrtho(0, LARGURA_DO_MUNDO, camera.bottom, camera.top, -1, 1);
    }

    float razaoAspectoJanela = (float)width/height;
    float razaoAspectoMundo = (float)LARGURA_DO_MUNDO/ALTURA_DO_MUNDO;

    if(razaoAspectoJanela < razaoAspectoMundo){
        float hViewport = (float) width / razaoAspectoMundo;
        float yViewport = ((float)height-hViewport)/2;

        camera.barraHorizontal = yViewport*2;
        camera.barraVertical = 0;
        glViewport(0, yViewport, width, hViewport);
    }
    else if(razaoAspectoJanela > razaoAspectoMundo){
        float wViewport = (float) height * razaoAspectoMundo;
        float xViewport = (float)(width-wViewport)/2;

        camera.barraHorizontal = 0;
        camera.barraVertical = xViewport*2;

        glViewport(xViewport, 0, wViewport, height);
    }else{
        camera.barraVertical = 0;
        camera.barraHorizontal = 0;
        glViewport(0, 0, width, height);
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void desenharRetanguloComTextura(double width, double height, GLuint textura)
{   
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textura);
    glBegin(GL_POLYGON);
    glTexCoord2f(0,1);
    glVertex3f(-width / 2 -.2, height / 2 +.2, 0);
    glTexCoord2f(1,1);
    glVertex3f(width / 2 +.2, height / 2 +.2, 0);
    glTexCoord2f(1,0);
    glVertex3f(width / 2 +.2, -height / 2 -.2, 0);
    glTexCoord2f(0,0);
    glVertex3f(-width / 2 -.2, -height / 2 -.2, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void inicializaProjetil(){
    if(projeteis == NULL)
        projeteis = malloc(1000 * sizeof(Projetil));
}

int criaProjetil(double x,double y, double velX, double velY, int id){
    int tempo = time(0);
    Projetil p = {id, x, y, 1, 1, velX, velY, 1, 1, tempo};
    for(int i = 0; i < quantidade_projeteis; i++){
        if(projeteis[i].existe == 0){
                projeteis[i] = p;
            return 0;
        }
    }
    projeteis[quantidade_projeteis] = p;
    quantidade_projeteis++;
    return 0;
}

int criaProjetilCircular(double x,double y, double velX, double velY, int id, double raio, double angulo, double xCentro, double yCentro){
    int tempo = time(0);
    Projetil p = {id, x, y, 1, 1, velX, velY, 1, 1, tempo, raio, angulo, xCentro, yCentro};
    for(int i = 0; i < quantidade_projeteis; i++){
        if(projeteis[i].existe == 0){
                projeteis[i] = p;
            return 0;
        }
    }
    projeteis[quantidade_projeteis] = p;
    quantidade_projeteis++;
    return 0;
}

void destroiProjetil(int indice){
    projeteis[indice].existe = 0;
    if(projeteis[indice].id == 7){
        for(int i = -1; i <= 1; i++){
            for(int j = -1; j <= 1; j++){
                if(i!= 0 || j !=0){
                criaProjetil(projeteis[indice].x, projeteis[indice].y,i,j,1);
                }
            }
        }
    }
}

int colidiuComPersonagem(double x, double y, double width, double height){
    return 
        x - width/2 > personagem.x - personagem.width/2 && 
        x + width/2 < personagem.x + personagem.width/2 &&
        y - height/2 > personagem.y - personagem.height/2 &&
        y + height/2 < personagem.y + personagem.height/2;
}

void atualizaCamera(){
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if(camera.arena){
        if(fase_atual == 3){
            if(camera.top >= (LINHAS_FASE-10)*sizeHeight){
                camera.arena = 0;
                boss.ativo = 1;
            }else{
                camera.y += 1;
            }
        }else if(fase_atual == 5){
            if(camera.top >= LINHAS_FASE*sizeHeight){
                camera.arena = 0;
                bossFinal.ativo = 1;
            }else{
                camera.y += 1;
            }
        }
    }

    //Camera sempre mira no personagem, mas só atualiza a camera se ele tiver subido
    int travou = 0;
    // for(int i = 0; i < quantidade_inimigos; i++){
    //     if(!inimigos[i].morreu && (inimigos[i].y - inimigos[i].height/2 - camera.bottom < 10)){
    //         travou = 1;
    //     }
    // }

    if(tela_atual == 2){
        if(!travou && !camera.travada )
            camera.y = personagem.y;
    }

    //O inicio da camera é a posição do personagem menos metade da altura, mantendo ele centralizado.
    camera.bottom = camera.y - (ALTURA_DO_MUNDO/2);

    //Não deixa que a camera filme antes de 0 ou depois do fim da fase.
    if(camera.bottom < 0){
        camera.bottom = 0;
    }else if(camera.bottom > (sizeHeight * LINHAS_FASE) - ALTURA_DO_MUNDO){
        camera.bottom = (sizeHeight * LINHAS_FASE) - ALTURA_DO_MUNDO;
    }
    camera.top = camera.bottom + ALTURA_DO_MUNDO;

    glOrtho(0, LARGURA_DO_MUNDO, camera.bottom, camera.top, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

typedef struct{
    double x, y, width, height, alfa;
    int comecou, acabou;
}AnimacaoGameOver;

AnimacaoGameOver gameover = {LARGURA_DO_MUNDO/2, 0, 160, 120, 0, 0, 0};

void desenhaGameOver(){
    glPushMatrix();
        glTranslatef(gameover.x, gameover.y, 1);
        glColor4f(1, 1, 1, gameover.alfa);
        desenharRetanguloComTextura(gameover.width, gameover.height, texturasMundo.gameover);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

void acabarGameOver(){
    gameover.acabou = 1;
}

void atualizaGameOver(){
    gameover.alfa += 0.01;

    if(gameover.acabou){
        tela_atual = 0;
        camera.y = ALTURA_DO_MUNDO/2;
        camera.travada = 0;
    }

    if(!gameover.acabou){
        glutTimerFunc(33, atualizaGameOver, 0);
    }
}

void gameOver(){
    gameover.y = camera.top - ALTURA_DO_MUNDO/2;
    gameover.comecou = 1;
    camera.travada = 1;
    personagem.travado = 1;

    glutTimerFunc(4000, acabarGameOver, 0);
    glutTimerFunc(33, atualizaGameOver, 0);
}

typedef struct{
    double x, y, width, height, alfa;
    int comecou, acabou;
}AnimacaoCreditos;

AnimacaoCreditos creditos = {LARGURA_DO_MUNDO/2, 0, 160, 400, 0, 0, 0};

void desenhaCreditos(){
    glColor4f(0.0, 0.0, 0.0, creditos.alfa);
    glPushMatrix();
        glTranslatef(LARGURA_DO_MUNDO/2, camera.bottom + ALTURA_DO_MUNDO/2, 0.95);
        desenharRetangulo(LARGURA_DO_MUNDO, ALTURA_DO_MUNDO);
    glPopMatrix();

    glColor4f(1.0, 1.0, 1.0, 1.0);
    glPushMatrix();
        glTranslatef(LARGURA_DO_MUNDO/2 - 10, creditos.y+92, 1);
        desenharRetanguloComTextura(10, 10, texturasMundo.vida1);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
        glTranslatef(creditos.x, creditos.y, 0.99);
        glColor4f(1, 1, 1, 1);
        desenharRetanguloComTextura(creditos.width, creditos.height, texturasMundo.creditos);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

void acabarCreditos(){
    creditos.acabou = 1;
}

void atualizaCreditos(){
    if(creditos.alfa < 1){
        creditos.alfa += 0.02;
    }

    if(creditos.y - creditos.height/2 < camera.bottom)
        creditos.y += 0.4;
    else{
        glutTimerFunc(3000, acabarCreditos, 0);
    }
        
    if(creditos.acabou){
        tela_atual = 0;
        fase_atual = 1;
        camera.y = ALTURA_DO_MUNDO/2;
        camera.travada = 0;
    }

    if(!creditos.acabou){
        glutTimerFunc(33, atualizaCreditos, 0);
    }
}

void subirCreditos(){
    creditos.y = camera.bottom - creditos.height/2 - 30;
    creditos.comecou = 1;
    creditos.acabou = 0;
    camera.travada = 1;
    personagem.travado = 1;

    glutTimerFunc(33, atualizaCreditos, 0);
}

void atualizaCorDeDanoPersonagem(){
    personagem.levouDano=0;
}

void causarDanoAoPersonagem(){
    if(!personagem.estaDefendendo && personagem.idPowerUp!=1){
        personagem.vidas--;

        if(game.pontuacao_atual > 10)
            game.pontuacao_atual -= 10;
        else{
            game.pontuacao_atual = 0;
        }
        personagem.levouDano=1;
        glutTimerFunc(100,atualizaCorDeDanoPersonagem,0);
    }
        
    if(personagem.vidas == 0)
        gameOver();
}

void atualizaProjetil(){
    int tempo = time(0);
    for(int i = 0; i < quantidade_projeteis; i++){
        int linha =  LINHAS_FASE-1 - (projeteis[i].y/sizeHeight);
        int coluna = projeteis[i].x/sizeWidth;
        double distanciaX = personagem.x - projeteis[i].x;
        double distanciaY = personagem.y - projeteis[i].y;
        double modulo = sqrt(distanciaX*distanciaX + distanciaY*distanciaY);
        if(projeteis[i].existe){
            if(projeteis[i].id == 2){
                projeteis[i].vel_X = distanciaX / modulo;
                projeteis[i].vel_Y = distanciaY / modulo;
                if(tempo - projeteis[i].timer >= 8)
                    destroiProjetil(i);
            }
            else if(projeteis[i].id == 4){
                if(fase[linha+1][coluna]!= 0){
                    destroiProjetil(i);
                }
            }
            else if(projeteis[i].id == 5){
                if(tempo - projeteis[i].timer >= 3){
                    destroiProjetil(i);
                }
            }else if(projeteis[i].id == 8){ //Teleguiado do Boss
                if(tempo - projeteis[i].timer >= 2){
                double Xatual = distanciaX/modulo;
                double Yatual = distanciaY/modulo;
                    projeteis[i].vel_X = Xatual;
                    projeteis[i].vel_Y = Yatual;  
                    projeteis[i].id = 1;
                }                
            }else if(projeteis[i].id == 9 || projeteis[i].id == 10){
                double xCentro = projeteis[i].xCentro;
                double yCentro = projeteis[i].yCentro;
                double raio = projeteis[i].raio;
                double angulo = projeteis[i].angulo;
                projeteis[i].x = (xCentro + raio*cos(angulo));
                projeteis[i].y = (yCentro + raio*sin(angulo));

                projeteis[i].xCentro += projeteis[i].vel_X;
                projeteis[i].yCentro += projeteis[i].vel_Y;

                projeteis[i].angulo -= 2*M_PI/60;

                if(projeteis[i].angulo <= 0)
                    projeteis[i].angulo += 2*M_PI;

            }
            else if(projeteis[i].id == 15){
                projeteis[i].vel_Y -= gravidade;
            }
            
            if(projeteis[i].id == 10){
                projeteis[i].raio += 1;
            }
            projeteis[i].x = projeteis[i].x + projeteis[i].vel_X;
            projeteis[i].y = projeteis[i].y + projeteis[i].vel_Y;

            if(projeteis[i].x >= LARGURA_DO_MUNDO || projeteis[i].y >= camera.top || projeteis[i].x <= 0 || projeteis[i].y <= camera.bottom){
                destroiProjetil(i);
            }
            if(colidiuComPersonagem(projeteis[i].x, projeteis[i].y, projeteis[i].width, projeteis[i].height)){
                destroiProjetil(i);
                causarDanoAoPersonagem();
            }
        }
    }
}

void desenhaCirculo(float raio, float x, float y, float z) {
    float angulo;

    glBegin(GL_TRIANGLE_FAN);
        glVertex3f(x, y, z);
        for (int i = 0; i <= 10; ++i) {
            angulo = 2 * M_PI * i / 10;
            glVertex3f(x + cos(angulo) * raio, y + sin(angulo) * raio, z);
        }
    glEnd();
}

void desenhaProjetil(){
    for(int i = 0; i < quantidade_projeteis; i++){
        if(projeteis[i].existe){
            
            // 1 mago roxo
            // 2 teleguiado do mago verde
            // 4 chuva do morcego
            //os do boss aq
            //15 parábola

            glPushMatrix();
            glTranslatef(projeteis[i].x, projeteis[i].y, .8);

            glColor3f(0,0,0);
            desenhaCirculo(1,projeteis[i].width, projeteis[i].height,0);

            switch(projeteis[i].id){
                case 1:
                    glColor3f(.4,0,.4);
                    break;
                case 2:
                    glColor3f(0,.6,0);
                    break;
                case 4:
                    glColor3f(0,.6,0);
                    break;
                case 5:
                    glColor3f(1,0,0);
                    break;
                case 6:
                    glColor3f(.2,.2,.2);
                    break;
                case 7:
                    glColor3f(.5,.5,.5);
                    break;
                case 8:
                    glColor3f(1,1,1);
                    break;
                case 9:
                    glColor3f(0.1, 0.1, 0.6);
                    break;
                case 10:
                    glColor3f(0.1, 0.6, 0.6);
                    break;
                case 15:
                    glColor3f(0,1,1);
                    break;
                default:
                    glColor3f(1,1,1);
                    break;
            }
            desenhaCirculo(.8,projeteis[i].width, projeteis[i].height, .1);
            glPopMatrix();
        }
    }
}

int criarInimigo(double x, double y, double width, double height, double velX, double velY, int vidas, int id){
    int tempo = time(0);
    Enemy inimigo = {x, y, width, height, velX, velY, vidas, 0, 0, id, tempo, 0};

    GLuint txt;

    if(id==1){
        txt = texturasInimigos.magoRoxo;
    }
    else if(id==2){
        txt = texturasInimigos.magoVerde;
    }
    else if(id == 4){
        txt = texturasInimigos.fantasma;
    }else if(id == 15){
        txt = texturasInimigos.magoMarrom;
    }
    else{
        if(id == 11){
            txt = texturasInimigos.morcego;
        }
        else{
            if(id == 9){
                txt = texturasInimigos.esquimo;
            }
        }
    }

    inimigo.textura = txt;
    for(int i = 0; i < quantidade_inimigos; i++){
        if(inimigos[i].morreu){
            inimigos[i] = inimigo;
            return 0;
        }
    }
    inimigos[quantidade_inimigos] = inimigo;
    quantidade_inimigos++;

    return 0;
}

void inicializarInimigos(){
    if(inimigos == NULL)
        inimigos = malloc(100*sizeof(Enemy));
}

void trataAnimacaoFantasma(Enemy* inimigo){
    inimigo->spriteY=0;
    if(inimigo->spriteX>=.944){
        inimigo->spriteX=0.03333;
    }
    else
        inimigo->spriteX+=.03333;
    return;
}

void trataAnimacaoMagos(Enemy* inimigo){
    if(!(inimigo->visao) && inimigo->velX){
        if(inimigo->velX<0){
            glRotatef(180,0,1,0);
        }
        inimigo->spriteY=.5;
        if(inimigo->spriteX>=.944){
            inimigo->spriteX=0.03333;
        }
        else
            inimigo->spriteX+=.03333;
            return;
    }

    if(inimigo->visao && (time(0)-inimigo->timer)==0){
        if(inimigo->x -personagem.x >0){
            glRotatef(180,0,1,0);
        }
        inimigo->spriteY=0;
        if(inimigo->spriteX>=.944){
            return;
        }
        else
            inimigo->spriteX+=.03333;
        return;
    }
    else{
        if(inimigo->velX<0){
            glRotatef(180,0,1,0);
        }
        inimigo->spriteY=.5;
        inimigo->spriteX=.03333;
    }
    
}

void desenhaMago(Enemy mago){
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, mago.textura);

    glBegin(GL_QUADS);   
    glTexCoord2f(mago.spriteX,mago.spriteY); 
        glVertex2f(-mago.width/2 , -mago.height/2);
    glTexCoord2f(mago.spriteX+.033,mago.spriteY);
        glVertex2f(mago.width/2, -mago.height/2);
    glTexCoord2f(mago.spriteX+.033,mago.spriteY+.499);
        glVertex2f(mago.width/2, mago.height/2);
    glTexCoord2f(mago.spriteX,mago.spriteY+.499);
        glVertex2f(-mago.width/2, mago.height/2);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void desenhaFantasma(Enemy fantasma){
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, fantasma.textura);
    
    glBegin(GL_QUADS);   
    glTexCoord2f(fantasma.spriteX,fantasma.spriteY); 
        glVertex2f(-fantasma.width/2 , -fantasma.height/2);
    glTexCoord2f(fantasma.spriteX+.033,fantasma.spriteY);
        glVertex2f(fantasma.width/2, -fantasma.height/2);
    glTexCoord2f(fantasma.spriteX+.033,fantasma.spriteY+1);
        glVertex2f(fantasma.width/2, fantasma.height/2);
    glTexCoord2f(fantasma.spriteX,fantasma.spriteY+1);
        glVertex2f(-fantasma.width/2, fantasma.height/2);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void trataAnimacaoMagoMarrom(Enemy* inimigo){
    if(time(0)-inimigo->timer ==0){
        if(inimigo->x -personagem.x >0){
            glRotatef(180,0,1,0);
        }
        inimigo->spriteY=0;
        if(inimigo->spriteX>=.944){
            return;
        }
        else
            inimigo->spriteX+=.03333;
        return;
    }
    else{
        if(inimigo->x -personagem.x >0){
            glRotatef(180,0,1,0);
        }
        inimigo->spriteY=.5;
        if(inimigo->spriteX>=.944){
            inimigo->spriteX=.03333;
            return;
        }
        else
            inimigo->spriteX+=.03333;
        return;
    }
}

void trataAnimacaoInimigoMorrendo(Enemy* inimigo){
    if(inimigo->direcao){
        glRotatef(180,0,1,0);
    }
    if(inimigo->spriteX>=.944){
        inimigo->morreu=1;
    }
    else
        inimigo->spriteX+=.03333;
}

void desenhaInimigoMorrendo(Enemy* inimigo, GLuint textura){
    trataAnimacaoInimigoMorrendo(inimigo);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textura);
    glBegin(GL_QUADS);   
    glTexCoord2f(inimigo->spriteX,0); 
        glVertex2f(-inimigo->width/2 , -inimigo->height/2);
    glTexCoord2f(inimigo->spriteX+.03333,0);
        glVertex2f(inimigo->width/2, -inimigo->height/2);
    glTexCoord2f(inimigo->spriteX+.03333,1);
        glVertex2f(inimigo->width/2, inimigo->height/2);
    glTexCoord2f(inimigo->spriteX,1);
        glVertex2f(-inimigo->width/2, inimigo->height/2);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void desenharInimigos(){
    for(int i = 0; i < quantidade_inimigos; i++){
        if(!inimigos[i].morreu){

            glPushMatrix();
            if(inimigos[i].levouDano){
                glColor3f(1, 0, 0);
                inimigos[i].levouDano=0;
            }
            else
                glColor3f(1, 1, 1);

            if(inimigos[i].id==15)
                glTranslatef(inimigos[i].x, inimigos[i].y +sizeHeight/2-1, 0.75);
            else{
                if(inimigos[i].id==2){
                    glTranslatef(inimigos[i].x, inimigos[i].y +sizeHeight/2-1, 0.79);
                }
                else{
                    if(inimigos[i].id ==1){
                        glTranslatef(inimigos[i].x, inimigos[i].y +sizeHeight/2-1, 0.78);
                    }
                    else{
                        if(inimigos[i].id == 4){
                            glTranslatef(inimigos[i].x, inimigos[i].y +sizeHeight/2-1, 0.78);
                        }
                        else{
                            if(inimigos[i].id == 9){
                                glTranslatef(inimigos[i].x, inimigos[i].y +sizeHeight/2-1, 0.77);
                            }
                            else{
                                if(inimigos[i].id == 11)
                                    glTranslatef(inimigos[i].x, inimigos[i].y +sizeHeight/2-1, 0.799);
                            }
                        }
                    }
                }
                    
            }
                
            glScalef(1.8,1.5,0);

            if(inimigos[i].id==1 || inimigos[i].id==2 || inimigos[i].id ==9){
                if(inimigos[i].x-personagem.x>0 && inimigos[i].vidas>0){
                    inimigos[i].direcao=1;
                }
                else{
                    if(inimigos[i].x-personagem.x<0 && inimigos[i].vidas>0)
                        inimigos[i].direcao=0;
                }
                    
                if(inimigos[i].vidas<=0){
                    inimigos[i].velX=0;
                    inimigos[i].visao=0;
                    if(inimigos[i].id==1)
                        desenhaInimigoMorrendo(&inimigos[i], texturasInimigos.roxoMorte);
                    else{
                        if(inimigos[i].id ==2){
                            desenhaInimigoMorrendo(&inimigos[i], texturasInimigos.verdeMorte);
                        }
                        else
                            desenhaInimigoMorrendo(&inimigos[i], texturasInimigos.esquimoMorte);
                    }
                }
                else{
                    trataAnimacaoMagos(&inimigos[i]);
                    desenhaMago(inimigos[i]);
                }
            }
            else{
                if(inimigos[i].id==15){
                    if(inimigos[i].vidas<=0){
                        inimigos[i].velX=0;
                        inimigos[i].visao=0;
                        desenhaInimigoMorrendo(&inimigos[i], texturasInimigos.marromMorte);
                    }
                    else{
                        trataAnimacaoMagoMarrom(&inimigos[i]);
                        desenhaMago(inimigos[i]);
                    }
                }
                else{
                    if(inimigos[i].id==4){
                        if(inimigos[i].vidas<=0){
                            inimigos[i].velX=0;
                            inimigos[i].visao=0;
                            glScalef(1.5,1.2,0);
                            desenhaInimigoMorrendo(&inimigos[i], texturasInimigos.fanstasmaMorte);
                        }
                        else{
                            trataAnimacaoFantasma(&inimigos[i]);
                            glScalef(1.5,1.2,0);
                            desenhaFantasma(inimigos[i]);
                        }   
                    }
                    else{
                        if(inimigos[i].id==11){
                            if(inimigos[i].vidas<=0){
                                inimigos[i].velX=0;
                                inimigos[i].velY=0;
                                inimigos[i].visao=0;
                                glScalef(1,.8,0);
                                desenhaInimigoMorrendo(&inimigos[i], texturasInimigos.morcegoMorte);
                            }
                            else{
                                glScalef(1,.8,0);
                                trataAnimacaoFantasma(&inimigos[i]);
                                desenhaFantasma(inimigos[i]);
                            }
                        }
                        
                    }
                }
            }
            glPopMatrix();
            glDisable(GL_TEXTURE_2D);
        }
    }
}

int inimigoSemChao(int i){
    int linha = (inimigos[i].y - inimigos[i].height/2)/sizeHeight;
    int coluna = 0;

    if(inimigos[i].velX > 0){
        coluna = (inimigos[i].x + inimigos[i].width/2)/sizeWidth;
    }
    else if(inimigos[i].velX < 0){
        coluna = (inimigos[i].x - inimigos[i].width/2)/sizeWidth;
    }

    if(fase[LINHAS_FASE-linha][coluna] == 0)
        return 1;

    return 0;
}

int inimigoBateuNaParede(int i){
    if(inimigos[i].x + inimigos[i].width/2 >= LARGURA_DO_MUNDO)
        return 1;
    if(inimigos[i].x - inimigos[i].width/2 <= 0)
        return 1;
    return 0;
}

void movimentacaoInimigos(int pos, int id){
    /*
    ID 1 - Atirador
    ID 2 - Atirador com tiro teleguiado
    ID 3 - Inimigo que segue
    ID 4 - Bombardeiro
    ID 5 - Passaro
    ID 8 - Boss
    */

    int tempo = time(0);
    int linha = (inimigos[pos].y) /sizeHeight;
    int coluna = (inimigos[pos].x) /sizeWidth;

    int distanciaX = personagem.x - inimigos[pos].x;
    int xPersonagem = distanciaX / fabs(distanciaX);

    int distanciaY = personagem.y - inimigos[pos].y;
    int yPersonagem = distanciaY / fabs(distanciaY);

    if(id == 1 || id == 2){
        if(linha == personagem.i){
            if((personagem.x - inimigos[pos].x < 0 && inimigos[pos].velX < 0) || (personagem.x - inimigos[pos].x > 0 && inimigos[pos].velX > 0)){
                inimigos[pos].visao = 1;
            }
        }
        else{
            inimigos[pos].visao = 0;   
        }
        if(inimigos[pos].visao == 0){
            inimigos[pos].x += inimigos[pos].velX;   
            if(inimigoSemChao(pos) || inimigoBateuNaParede(pos)){
                inimigos[pos].velX *= -1;
            }                 
        }
    }
    else if(id == 3){
        if(linha == personagem.i){
            if((personagem.x - inimigos[pos].x < 0 && inimigos[pos].velX < 0) || (personagem.x - inimigos[pos].x > 0 && inimigos[pos].velX > 0)){
                inimigos[pos].visao = 1;
            } 
        }
        if(abs(distanciaY) >= 150){
            inimigos[pos].visao = 0;
        }
        if(time(0)%4 == 0){
            if(inimigos[pos].visao == 1){
                if(!inimigoSemChao(pos) && !inimigoBateuNaParede(pos)){
                    inimigos[pos].velX = xPersonagem;
                    inimigos[pos].x += inimigos[pos].velX;
                }
                if((inimigoSemChao(pos) || inimigoBateuNaParede(pos))){
                    if((distanciaX > 0 && inimigos[pos].velX < 0) || (distanciaX < 0 && inimigos[pos].velX > 0)){
                        inimigos[pos].velX = inimigos[pos].velX * -1;
                        inimigos[pos].x += inimigos[pos].velX;
                    }
                }
            }
            if(inimigos[pos].visao == 0){
                inimigos[pos].x += inimigos[pos].velX; 
                    if(inimigoSemChao(pos) || inimigoBateuNaParede(pos)){
                        inimigos[pos].velX *= -1;
                    }                      
            }
        }
    }
    else if(id == 4){
        if(distanciaY >= -40){
            inimigos[pos].x += inimigos[pos].velX;
            inimigos[pos].visao = 1;
        }
        else if(inimigos[pos].visao == 1){
            inimigos[pos].x += inimigos[pos].velX;
        }

        if(inimigos[pos].x > LARGURA_DO_MUNDO || inimigos[pos].x < 0)
            inimigos[pos].morreu = 1;
    }
    else if(id == 9){
        if(fabs(distanciaX) < 100 && fabs(distanciaY) < 100){
            inimigos[pos].visao = 1;
        }
    }
    else if(id == 11){
        if(fabs(distanciaY) < 100 && !inimigos[pos].vidas<=0 && (time(0) - inimigos[pos].timer>0)){

            inimigos[pos].visao = 1;
            double modulo = sqrt(distanciaX*distanciaX + distanciaY *distanciaY);

            inimigos[pos].velX = distanciaX/modulo;
            inimigos[pos].velY = distanciaY/modulo;

            inimigos[pos].x += inimigos[pos].velX;
            inimigos[pos].y += inimigos[pos].velY;

            double width = inimigos[pos].width;
            double height = inimigos[pos].height;
            if(
                colidiuComPersonagem(inimigos[pos].x - width/2, inimigos[pos].y + height/2, 0, 0) ||
                colidiuComPersonagem(inimigos[pos].x + width/2, inimigos[pos].y + height/2, 0, 0) ||
                colidiuComPersonagem(inimigos[pos].x + width/2, inimigos[pos].y - height/2, 0, 0) ||
                colidiuComPersonagem(inimigos[pos].x - width/2, inimigos[pos].y - height/2, 0, 0)
            ){
                if(personagem.estaDefendendo){
                    inimigos[pos].x -= 30*inimigos[pos].velX;
                    inimigos[pos].y -= 30*inimigos[pos].velY;
                    inimigos[pos].timer = time(0)+1;
                }else{
                    inimigos[pos].x -= 10*inimigos[pos].velX;
                    inimigos[pos].y -= 10*inimigos[pos].velY;
                    inimigos[pos].timer = time(0);
                }
                causarDanoAoPersonagem();
            }
    }

    }
    else if(id == 15){
        if(fabs(distanciaX) < 100 && fabs(distanciaX) > 20 && fabs(distanciaY) < 100){
            inimigos[pos].visao = 1;
        }else{
            inimigos[pos].x += inimigos[pos].velX;   
            if(inimigoSemChao(pos) || inimigoBateuNaParede(pos)){
                inimigos[pos].velX *= -1;
            }
        }
    }
}

void gerenciamentoProjetil(int pos, int id){
    int tempo = time(0);
    double distancia = personagem.x - inimigos[pos].x;
    int direcao = distancia / fabs(distancia);
    int tempoDeRecarga, estaVendo;
    estaVendo = inimigos[pos].visao;

    if(id == 1){
        tempoDeRecarga = 3;
    }else if(id == 2){
        tempoDeRecarga = 4;
        estaVendo = 1;
    }else if (id == 9){
        tempoDeRecarga = 3;
    }
    else if(id == 15){
        tempoDeRecarga = 3;
    }else if(id == 4){
        if((int)inimigos[pos].x % (int)sizeWidth == 0 && inimigos[pos].visao){
            criaProjetil(inimigos[pos].x, inimigos[pos].y - inimigos[pos].height/2, 0, -1, inimigos[pos].id);
        }
    }
    if(id == 1 || id == 2 || id == 9 || id == 15 || id == 25){
        if(inimigos[pos].visao == 1){
            if(tempo - inimigos[pos].timer >= tempoDeRecarga){
                if(id == 1 || id == 2){
                    criaProjetil(inimigos[pos].x - inimigos[pos].width/2, inimigos[pos].y, direcao, 0, inimigos[pos].id);
                    inimigos[pos].timer = tempo;
                }else if(id == 15){
                    double t = 25;
                    double dX = personagem.x - inimigos[pos].x;
                    double dY = personagem.y - inimigos[pos].y;
                    double velocidadeInicialX = dX/t;
                    double velocidadeInicialY = 0;

                    velocidadeInicialY = (dY + ((gravidade*t*t)/2))/t;
                    criaProjetil(
                        inimigos[pos].x, 
                        inimigos[pos].y, 
                        velocidadeInicialX, 
                        velocidadeInicialY, 
                        inimigos[pos].id
                    );
                    inimigos[pos].timer = tempo;
                }else if(id == 9){
                    for(int i = 0; i < 7; i++){
                        double dX = 4*i*i;
                        double dY = 4*i*i;
                        //double raio = sqrt(dX + dY);
                        double raio = 3;
                        double velX = personagem.x - inimigos[pos].x;
                        double velY = personagem.y - inimigos[pos].y;
                        double modulo = sqrt((velX * velX) + (velY * velY));
                        velX /= modulo;
                        velY /= modulo;
                        criaProjetilCircular(
                            inimigos[pos].x + i*2, 
                            inimigos[pos].y - i*2, 
                            velX, velY, inimigos[pos].id, 
                            raio, 
                            (2*M_PI)/7 * i, 
                            inimigos[pos].x, inimigos[pos].y);
                    }
                    inimigos[pos].timer = tempo;
                }
            }
        }
    }
}

void trataAnimacaoDoBoss(){
    if(time(0)-boss.timerAtaque3 == 0 || time(0)-boss.timerBomba == 0 || time(0)-boss.timerMovimento == 0 || time(0)-boss.timerTeleguiado == 0){
        boss.spriteY=0;
        if(boss.spriteX>.26664){
            boss.spriteX=0;
        }
        else
            boss.spriteX+=.03333;
        return;
    }
    else{
        boss.spriteY=.5;
        if(boss.spriteX>=.944){
            boss.spriteX=0.03333;
        }
        else
            boss.spriteX+=.03333;
        return;
    }
}

void desenharBoss(){
    trataAnimacaoDoBoss();

    if(boss.levouDano){
        glColor3f(1, 0 ,0);
    }
    else
        glColor3f(1, 1 ,1);

    glPushMatrix();
        glTranslatef(boss.x , boss.y, .8);
        glRotatef(boss.angulo, 0, 0, 1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, boss.textura);
        glBegin(GL_QUADS);
            glTexCoord2f(boss.spriteX,boss.spriteY); 
                glVertex2f(-boss.width , -boss.height);
            glTexCoord2f(boss.spriteX+.033,boss.spriteY);
                glVertex2f(boss.width, -boss.height);
            glTexCoord2f(boss.spriteX+.033,boss.spriteY+.48);
                glVertex2f(boss.width, boss.height );
            glTexCoord2f(boss.spriteX,boss.spriteY+.48);
                glVertex2f(-boss.width, boss.height);
        glEnd();
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void atualizarBoss(){

    double distancia = personagem.x - boss.x;
    int direcao = distancia / fabs(distancia);

    if(boss.ativo && boss.existe){
        int tempo = time(0);
        if(tempo - boss.timerBomba >=7){ // A cada 4 segundos: cria uma bomba
            criaProjetil(boss.x - boss.width/2,boss.y, direcao * 0.5, 0, 7);
            boss.timerBomba = tempo;
        }else if(tempo - boss.timerTeleguiado >= 6){
            criaProjetil(boss.x, boss.y + boss.height/2, 0.4, 0.6 , 8);
            criaProjetil(boss.x, boss.y + boss.height/2, -0.4, 0.6 , 8);
            criaProjetil(boss.x, boss.y + boss.height/2, -0.2, 0.8 , 8);
            criaProjetil(boss.x, boss.y + boss.height/2, 0.2, 0.8, 8);
            boss.timerTeleguiado = tempo;
        }
    }
}

void atualizarInimigos(){
    for(int i = 0; i < quantidade_inimigos; i++){
        if(!inimigos[i].morreu){
            movimentacaoInimigos(i, inimigos[i].id);
            gerenciamentoProjetil(i, inimigos[i].id);
        }
    }
}

void desenharMenu()
{
    glColor3f(1,1,1);

    // Fundo do menu
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturasMundo.menu);
    glTranslatef(LARGURA_DO_MUNDO / 2, ALTURA_DO_MUNDO / 2, 0);
    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0,1);
        glVertex3f(-LARGURA_DO_MUNDO / 2, ALTURA_DO_MUNDO / 2, 0);
    glTexCoord2f(1,1);
        glVertex3f(LARGURA_DO_MUNDO / 2, ALTURA_DO_MUNDO / 2, 0);
    glTexCoord2f(1,0);
        glVertex3f(LARGURA_DO_MUNDO / 2, -ALTURA_DO_MUNDO / 2, 0);
    glTexCoord2f(0,0);
        glVertex3f(-LARGURA_DO_MUNDO / 2, -ALTURA_DO_MUNDO / 2, 0);
    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);

    // Botões
    for (int i = 0; i < quantidade_botoes; i++)
    {
        Button btn = *buttons[i];
        if (btn.id / 10 == tela_atual)
        {
            glColor3f(btn.cores[0], btn.cores[1], btn.cores[2]);
            glPushMatrix();
            glTranslatef(btn.x, btn.y, 1);

            GLuint aux;
            if(btn.id==1){
                aux = texturasMundo.botaoIniciar;
            }
            else{
                if(btn.id==2){
                    aux = texturasMundo.botaoControles;
                }
            }
            desenharRetanguloComTextura(btn.width, btn.height, aux);
            glPopMatrix();
        }
    }

    glColor3f(1.0, 1.0, 1.0);
    char *texto = malloc(30*sizeof(char));
    sprintf(texto, "Melhor Pontuacao: %d", game.maior_pontuacao);
    escreveTexto(GLUT_BITMAP_8_BY_13, texto, 10, ALTURA_DO_MUNDO/2, 1);
    free(texto);
}

void inicializaTexturas(){

    texturasMundo.vida1 = carregaTextura("imagens/extras/vidas1.png");
    texturasMundo.vida2 = carregaTextura("imagens/extras/vidas2.png");
    texturasMundo.gameover = carregaTextura("imagens/extras/gameover.png");

    texturasMundo.grama[0] = carregaTextura("imagens/1/grama.png");
    texturasMundo.plataforma[0] = carregaTextura("imagens/1/plataforma.png");
    texturasMundo.fundo0[0] = carregaTextura("imagens/1/fundo0.png");
    texturasMundo.fundo1[0] = carregaTextura("imagens/1/fundo1.png");
    texturasMundo.fundo2[0] = carregaTextura("imagens/1/fundo2.png");

    texturasMundo.grama[1] = carregaTextura("imagens/2/grama.png");
    texturasMundo.plataforma[1] = carregaTextura("imagens/2/plataforma.png");
    texturasMundo.fundo0[1] = carregaTextura("imagens/2/fundo0.png");
    texturasMundo.fundo1[1] = carregaTextura("imagens/2/fundo1.png");
    texturasMundo.fundo2[1] = carregaTextura("imagens/2/fundo2.png");

    texturasMundo.grama[2] = carregaTextura("imagens/3/grama.png");
    texturasMundo.plataforma[2] = carregaTextura("imagens/3/plataforma.png");
    texturasMundo.fundo0[2] = carregaTextura("imagens/3/fundo0.png");
    texturasMundo.fundo1[2] = carregaTextura("imagens/3/fundo1.png");
    texturasMundo.fundo2[2] = 0;

    texturasMundo.grama[3] = carregaTextura("imagens/4/grama.png");
    texturasMundo.plataforma[3] = carregaTextura("imagens/4/plataforma.png");
    texturasMundo.fundo0[3] = carregaTextura("imagens/4/fundo0.png");
    texturasMundo.fundo1[3] = carregaTextura("imagens/4/fundo1.png");
    texturasMundo.fundo2[3] = carregaTextura("imagens/4/fundo2.png");

    texturasMundo.grama[4] = carregaTextura("imagens/5/grama.png");
    texturasMundo.plataforma[4] = carregaTextura("imagens/5/plataforma.png");
    texturasMundo.fundo0[4] = carregaTextura("imagens/5/fundo0.png");
    texturasMundo.fundo1[4] = carregaTextura("imagens/5/fundo1.png");
    texturasMundo.fundo2[4] = 0;


    texturasMundo.blocoDeslizante =  carregaTextura("imagens/4/blocoDeslizante.png");  
    texturasMundo.masmorra = carregaTextura("imagens/5/masmorra.png");  

    boss.textura = carregaTextura("imagens/boss.png");
    bossFinal.textura= carregaTextura("imagens/bossFinal.png");

    texturasInimigos.magoMarrom=carregaTextura("imagens/magoMarrom.png");
    texturasInimigos.magoRoxo=carregaTextura("imagens/magoRoxo.png");
    texturasInimigos.magoVerde=carregaTextura("imagens/magoVerde.png");
    texturasInimigos.fantasma = carregaTextura("imagens/fantasma.png");
    texturasInimigos.morcego = carregaTextura("imagens/morcego.png");
    texturasInimigos.esquimo = carregaTextura("imagens/esquimo.png");
    
    texturasInimigos.fanstasmaMorte = carregaTextura("imagens/fantasmaMorte.png");
    texturasInimigos.marromMorte = carregaTextura("imagens/magoMarromMorte.png");
    texturasInimigos.roxoMorte = carregaTextura("imagens/magoRoxoMorte.png");
    texturasInimigos.verdeMorte = carregaTextura("imagens/magoVerdeMorte.png");
    texturasInimigos.morcegoMorte = carregaTextura("imagens/morcegoMorte.png");
    texturasInimigos.esquimoMorte = carregaTextura("imagens/esquimoMorte.png");   

    texturasMundo.menu = carregaTextura("imagens/extras/menu.png");
    texturasMundo.botaoIniciar = carregaTextura("imagens/extras/iniciar.png");
    texturasMundo.botaoControles = carregaTextura("imagens/extras/controles.png");
    texturasMundo.telaControles = carregaTextura("imagens/extras/telaControles.png");
    texturasMundo.botaoVoltar = carregaTextura("imagens/extras/voltar.png");

    texturasMundo.powerUpArmadura = carregaTextura("imagens/extras/powerUpArmadura.png");
    texturasMundo.vidasPowerUp = carregaTextura("imagens/extras/vidasPowerUp.png");

    portal.textura = carregaTextura("imagens/extras/portal.png");

    texturasMundo.creditos = carregaTextura("imagens/extras/creditos.png");
}

void inicializarRounds(){  
    Round round1 = {1, 4, {1, 2}, 2, {35, 11, 6, 24, 34}, {28, 23, 15, 14, 21}, 5, 0, 2, 0, 28};
    Round round2 = {2, 4, {1, 2, 15}, 3, {20, 30, 5, 35, 28}, {53, 50, 60, 60, 42}, 5, 0, 2, 0, 60};
    Round round3 = {3, 10, {1, 2, 4, 15}, 4, {5, 35, 2, 32, 20}, {53, 53, 47, 47, 50}, 5, 0, 2, 0, 59};

    round1.alturaMinima = (LINHAS_FASE - 28)*sizeHeight;
    round2.alturaMinima = (LINHAS_FASE - 60)*sizeHeight;
    round3.alturaMinima = (LINHAS_FASE - 59)*sizeHeight;

    rounds[0] = round1;
    rounds[1] = round2;
    rounds[2] = round3;
}

void lerFase(char *arquivo, int lerInimigos)
{
    FILE *arquivo_fase;
    arquivo_fase = fopen(arquivo, "r");

    for (int i = 0; i < LINHAS_FASE; i++)
    {
        for (int j = 0; j < COLUNAS_FASE; j++)
        {
            fscanf(arquivo_fase, "%d", &fase[i][j]);
        }
    }


    //Le inimigos
    if(lerInimigos){
        for(int i = 0; i < QUANTIDADE_INIMIGOS[fase_atual-1]; i++){
            int x, y, id;
            double width, height;
            fscanf(arquivo_fase, "%d", &x);
            fscanf(arquivo_fase, "%d", &y);
            fscanf(arquivo_fase, "%d", &id);
            width = 5;
            height = 8;

            criarInimigo(x*sizeWidth, (LINHAS_FASE-y)*sizeHeight + height/2, width, height, 1, 0, 2, id);
        }
    }

    fclose(arquivo_fase);
}

void configuraAssetsFase1(){
    portal.x=41;
    portal.y=582;
    portal.existe=1;

    powerUps[0].id = 2;
    powerUps[0].existe = 1;
    powerUps[0].x = 32 * sizeWidth;
    powerUps[0].y = (LINHAS_FASE - 78) * sizeHeight;

    powerUps[1].id = 2;
    powerUps[1].existe = 1;
    powerUps[1].x = 8 * sizeWidth;
    powerUps[1].y = (LINHAS_FASE - 47) * sizeHeight;
}

void configuraAssetsFase2(){
    portal.existe=1;
    portal.x=26;
    portal.y=576;

    for(int i=0; i<10; i++)
        powerUps[i].existe=0;

    powerUps[10].id = 1;
    powerUps[10].existe = 1;
    powerUps[10].x = 9;
    powerUps[10].y = 245;

    powerUps[11].id = 2;
    powerUps[11].existe = 1;
    powerUps[11].x = LARGURA_DO_MUNDO/2;
    powerUps[11].y = 125;

    powerUps[12].id = 1;
    powerUps[12].existe = 1;
    powerUps[12].x = 138;
    powerUps[12].y = 503;
}

void configuraAssetsFase3(){
    portal.existe=0;
    boss.vida = MAX_VIDAS_BOSS;
    boss.existe = 1;
    boss.ativo = 0;
    portal.x=137;
    portal.y=432;

    for(int i=0; i<20; i++)
        powerUps[i].existe=0;

    powerUps[20].id = 2;
    powerUps[20].existe = 1;
    powerUps[20].x = 20 * sizeWidth;
    powerUps[20].y = (LINHAS_FASE - 58) * sizeHeight;

    powerUps[21].id = 2;
    powerUps[21].existe = 1;
    powerUps[21].x = 20 * sizeWidth;
    powerUps[21].y = (LINHAS_FASE - 46) * sizeHeight;

    powerUps[22].id = 2;
    powerUps[22].existe = 1;
    powerUps[22].x = 13 * sizeWidth;
    powerUps[22].y = (LINHAS_FASE - 32) * sizeHeight;

    powerUps[23].id = 2;
    powerUps[23].existe = 1;
    powerUps[23].x = 27 * sizeWidth;
    powerUps[23].y = (LINHAS_FASE - 32) * sizeHeight;
}

void configuraAssetsFase4(){
    portal.existe=1;
    portal.x=77;
    portal.y=492;

    for(int i=0; i<30; i++)
        powerUps[i].existe=0;
    
    powerUps[30].id = 2;
    powerUps[30].existe = 1;
    powerUps[30].x = 79;
    powerUps[30].y = 71;

    powerUps[31].id = 1;
    powerUps[31].existe = 1;
    powerUps[31].x = 8;
    powerUps[31].y = 251;

    powerUps[32].id = 2;
    powerUps[32].existe = 1;
    powerUps[32].x = 16;
    powerUps[32].y = 383;

    powerUps[33].existe = 1;
    powerUps[33].id = 2;
    powerUps[33].x = 144;
    powerUps[33].y = 383;
}

void configuraAssetsFase5(){
    portal.existe=0;
    portal.x=77;
    portal.y=492;
    bossFinal.vida = MAX_VIDAS_BOSS;
    bossFinal.existe = 1;
    bossFinal.x = LARGURA_DO_MUNDO/2;
    bossFinal.y = 520;

    for(int i=0; i<40; i++)
        powerUps[i].existe=0;

    powerUps[40].id = 2;
    powerUps[40].existe = 1;
    powerUps[40].x = 10;
    powerUps[40].y = 167;

    powerUps[41].id = 1;
    powerUps[41].existe = 1;
    powerUps[41].x = 31;
    powerUps[41].y = 317;
    
}

void atualizarObstaculos(){
    for(int i = 0; i < quantidade_obstaculos; i++){
        if(obstaculos[i].fase == fase_atual){
            //Rotaciona obstaculoss
            if(obstaculos[i].sentido == 1){
                obstaculos[i].angulo += M_PI/50;
                if(obstaculos[i].angulo >= 2*M_PI)
                    obstaculos[i].angulo -=  2*M_PI;
            }else{
                obstaculos[i].angulo -= M_PI/50;
                if(obstaculos[i].angulo <= 0)
                    obstaculos[i].angulo +=  2*M_PI;
            }

            //Checa colisão com personagem
            double distanciaX = personagem.x - obstaculos[i].xCentro;
            double distanciaY = personagem.y - obstaculos[i].yCentro;

            double obstaculoX = obstaculos[i].raio * cos(obstaculos[i].angulo);
            double obstaculoY = obstaculos[i].raio * sin(obstaculos[i].angulo);

            double numerador = distanciaX * obstaculoX + distanciaY*obstaculoY;

            double normaPersonagem = sqrt(distanciaX*distanciaX + distanciaY*distanciaY);
            double normaObstaculo = sqrt(obstaculoX*obstaculoX + obstaculoY*obstaculoY);

            double denominador = normaPersonagem * normaObstaculo;
            double semelhanca = numerador/denominador;

            if(semelhanca >= 0.99 && normaObstaculo >= normaPersonagem){
                causarDanoAoPersonagem();
            }
        }
    }
}

void desenharObstaculos(){
    for(int i = 0; i < quantidade_obstaculos; i++){
        if(obstaculos[i].fase == fase_atual){
            double quantidade = obstaculos[i].raio / 3;
            float tom = 0.2;
            for(int j = 0; j < quantidade; j++){
                glColor3f(tom, 0.2, 0.2);
                if(tom <= 0.95)
                    tom += 0.05;
                glPushMatrix();
                    glTranslatef(obstaculos[i].xCentro, obstaculos[i].yCentro, .8);
                    double x = ((j*3)+1.5) * cos(obstaculos[i].angulo);
                    double y = ((j*3)+1.5) * sin(obstaculos[i].angulo);
                    desenhaCirculo(1.5, x, y, .05);
                    glColor3f(0,0,0);
                    desenhaCirculo(1.8, x, y, .04);
                glPopMatrix();
            }
        }
    }
}

void inicializarObstaculos(){
    obstaculos[0].fase = 4;
    obstaculos[0].xCentro = 20*sizeWidth;
    obstaculos[0].yCentro = (LINHAS_FASE-45.5)*sizeHeight; ;
    obstaculos[0].raio = 54;
    obstaculos[0].angulo = 0;
    obstaculos[0].sentido = 1;

    obstaculos[1].fase = 4;
    obstaculos[1].xCentro = 16*sizeWidth;
    obstaculos[1].yCentro = (LINHAS_FASE-33.5)*sizeHeight; ;
    obstaculos[1].raio = 54;
    obstaculos[1].angulo = 0;
    obstaculos[1].sentido = -1;

    obstaculos[2].fase = 4;
    obstaculos[2].xCentro = 24*sizeWidth;
    obstaculos[2].yCentro = (LINHAS_FASE-33.5)*sizeHeight; ;
    obstaculos[2].raio = 54;
    obstaculos[2].angulo = M_PI;
    obstaculos[2].sentido = 1;

    obstaculos[3].fase = 3;
    obstaculos[3].xCentro = 6.5*sizeWidth;
    obstaculos[3].yCentro = (LINHAS_FASE-17.5)*sizeHeight;
    obstaculos[3].raio = 54;
    obstaculos[3].angulo = 0;
    obstaculos[3].sentido = 1;

    obstaculos[4].fase = 3;
    obstaculos[4].xCentro = 33.5*sizeWidth;
    obstaculos[4].yCentro = (LINHAS_FASE-17.5)*sizeHeight;
    obstaculos[4].raio = 54;
    obstaculos[4].angulo = M_PI;
    obstaculos[4].sentido = -1;
}

void inicializarFase(){
    gameover.acabou = gameover.comecou = 0;
    gameover.alfa = 0;

    quantidade_inimigos = 0;
    quantidade_projeteis = 0;

    char *arquivo = malloc(20*sizeof(char));
    sprintf(arquivo, "fases/fase%d.txt", fase_atual);
    lerFase(arquivo, 1);

    Personagem p = {10, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, MAX_VIDAS, personagem.textura, 0, 0, 0};
    personagem = p;
    personagem.podeAtacar=1;
    personagem.vidas=MAX_VIDAS;
    personagem.y=sizeHeight*3;

    camera.y = ALTURA_DO_MUNDO/2;
    camera.travada = 0;
    camera.arena = 0;


    if(fase_atual==1){
        configuraAssetsFase1();
    }

    if(fase_atual==2){
        configuraAssetsFase2();
    }

    if(fase_atual == 3){
        configuraAssetsFase3();
    }

    if(fase_atual == 4){
        configuraAssetsFase4();
    }

    if(fase_atual ==5){
        configuraAssetsFase5();
    }
    
    inicializarObstaculos();
    inicializarRounds();

    free(arquivo);
}

void criaFundoLista1(){
    glColor4f(1,1,1,1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturasMundo.fundo0[0]);
    glBegin(GL_POLYGON);
        glTexCoord2f(0,0);
            glVertex3f(0,0, -.5);
        glTexCoord2f(1,0);
            glVertex3f(LARGURA_DO_MUNDO,0, -.5);
        glTexCoord2f(1,1);
            glVertex3f(LARGURA_DO_MUNDO,ALTURA_DO_MUNDO*5, -.5);
        glTexCoord2f(0,1);
            glVertex3f(0, ALTURA_DO_MUNDO*5, -.5);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void criaFundoLista2(){
    glColor4f(1,1,1,1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturasMundo.fundo0[1]);
    glBegin(GL_POLYGON);
        glTexCoord2f(0,0);
            glVertex3f(0,0, -.5);
        glTexCoord2f(1,0);
            glVertex3f(LARGURA_DO_MUNDO,0, -.5);
        glTexCoord2f(1,1);
            glVertex3f(LARGURA_DO_MUNDO,ALTURA_DO_MUNDO*5, -.5);
        glTexCoord2f(0,1);
            glVertex3f(0, ALTURA_DO_MUNDO*5, -.5);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void criaFundoLista3(){
    glColor3f(.5,.5,.5);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturasMundo.fundo0[2]);
    for(int i=0; i<ALTURA_DO_MUNDO*5; i+=ALTURA_DO_MUNDO){
        glBegin(GL_POLYGON);
            glTexCoord2f(0,0);
                glVertex3f(0,i, -.8);
            glTexCoord2f(1,0);
                glVertex3f(LARGURA_DO_MUNDO,i, -.8);
            glTexCoord2f(1,1);
                glVertex3f(LARGURA_DO_MUNDO,ALTURA_DO_MUNDO+i, -.8);
            glTexCoord2f(0,1);
                glVertex3f(0, ALTURA_DO_MUNDO+i, -.8);
        glEnd();
    }
    glDisable(GL_TEXTURE_2D);
    
}

void criaFundoLista4(){
    glColor4f(1,1,1,1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturasMundo.fundo0[3]);
    glBegin(GL_POLYGON);
        glTexCoord2f(0,0);
            glVertex3f(0,0, -.5);
        glTexCoord2f(1,0);
            glVertex3f(LARGURA_DO_MUNDO,0, -.5);
        glTexCoord2f(1,1);
            glVertex3f(LARGURA_DO_MUNDO,ALTURA_DO_MUNDO*5, -.5);
        glTexCoord2f(0,1);
            glVertex3f(0, ALTURA_DO_MUNDO*5, -.5);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_2D);
}

void criaFundoLista5(){
    glColor4f(1,1,1,1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturasMundo.fundo0[4]);
    glBegin(GL_POLYGON);
        glTexCoord2f(0,0);
            glVertex3f(0,0, -.5);
        glTexCoord2f(1,0);
            glVertex3f(LARGURA_DO_MUNDO,0, -.5);
        glTexCoord2f(1,1);
            glVertex3f(LARGURA_DO_MUNDO,ALTURA_DO_MUNDO*5, -.5);
        glTexCoord2f(0,1);
            glVertex3f(0, ALTURA_DO_MUNDO*5, -.5);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_2D);
}

void criaListasFases() {
    char *arquivo = malloc(20*sizeof(char));
    for(int l = 0; l < 5; l++){
        listas_fases[l] = glGenLists(1);
        glNewList(listas_fases[l], GL_COMPILE);  

        sprintf(arquivo, "fases/fase%d.txt", l+1);
        lerFase(arquivo, 0);
        if(l==0){
            criaFundoLista1();
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texturasMundo.grama[0]);
            glBegin(GL_POLYGON);
                glTexCoord2f(0,0);
                    glVertex2f(0,0);
                glTexCoord2f(1,0);
                    glVertex2f(LARGURA_DO_MUNDO,0);
                glTexCoord2f(1,1);
                    glVertex2f(LARGURA_DO_MUNDO,sizeHeight*2);
                glTexCoord2f(0,1);
                    glVertex2f(0, sizeHeight*2);
            glEnd();
            glDisable(GL_TEXTURE_2D);
            
        }
        if(l==1){
            criaFundoLista2();
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texturasMundo.grama[1]);
            glBegin(GL_POLYGON);
                glTexCoord2f(0,0);
                    glVertex2f(0,0);
                glTexCoord2f(1,0);
                    glVertex2f(LARGURA_DO_MUNDO,0);
                glTexCoord2f(1,1);
                    glVertex2f(LARGURA_DO_MUNDO,sizeHeight*2);
                glTexCoord2f(0,1);
                    glVertex2f(0, sizeHeight*2);
            glEnd();
            glDisable(GL_TEXTURE_2D);
        }
        if(l==2){
            criaFundoLista3();
            glColor3f(1,1,1);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texturasMundo.grama[2]);
            glBegin(GL_POLYGON);
                glTexCoord2f(0,0);
                    glVertex2f(0,0);
                glTexCoord2f(1,0);
                    glVertex2f(LARGURA_DO_MUNDO,0);
                glTexCoord2f(1,1);
                    glVertex2f(LARGURA_DO_MUNDO,sizeHeight*2);
                glTexCoord2f(0,1);
                    glVertex2f(0, sizeHeight*2);
            glEnd();
            glDisable(GL_TEXTURE_2D);
        }

        if(l==3){
            criaFundoLista4();
            glColor3f(1,1,1);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texturasMundo.grama[3]);
            glBegin(GL_POLYGON);
                glTexCoord2f(0,0);
                    glVertex3f(0,0,.5);
                glTexCoord2f(1,0);
                    glVertex3f(LARGURA_DO_MUNDO,0,.5);
                glTexCoord2f(1,1);
                    glVertex3f(LARGURA_DO_MUNDO,sizeHeight*2,.5);
                glTexCoord2f(0,1);
                    glVertex3f(0, sizeHeight*2,.5);
            glEnd();
            glDisable(GL_TEXTURE_2D);
        }
        if(l==4){
            criaFundoLista5();
            glColor3f(1,1,1);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texturasMundo.grama[4]);
            glBegin(GL_POLYGON);
                glTexCoord2f(0,0);
                    glVertex3f(0,0,.5);
                glTexCoord2f(1,0);
                    glVertex3f(LARGURA_DO_MUNDO,0,.5);
                glTexCoord2f(1,1);
                    glVertex3f(LARGURA_DO_MUNDO,sizeHeight*2,.5);
                glTexCoord2f(0,1);
                    glVertex3f(0, sizeHeight*2,.5);
            glEnd();
            glDisable(GL_TEXTURE_2D);

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texturasMundo.masmorra);
            glBegin(GL_POLYGON);
                glTexCoord2f(0,0);
                    glVertex3f(0,480,-.49);
                glTexCoord2f(1,0);
                    glVertex3f(LARGURA_DO_MUNDO,480,-.49);
                glTexCoord2f(1,1);
                    glVertex3f(LARGURA_DO_MUNDO,600,-.49);
                glTexCoord2f(0,1);
                    glVertex3f(0, 600,-.49);
            glEnd();
            glDisable(GL_TEXTURE_2D);
        }
        for (int i = 0; i < LINHAS_FASE; i++)
        {
            for (int j = 0; j < COLUNAS_FASE; j++)
            {
                int linha_atual = LINHAS_FASE-1-i;
                if (fase[linha_atual][j] == 1)
                {
                    glColor3f(1, 1, 1);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glPushMatrix();
                        glTranslatef(j * sizeWidth + sizeWidth / 2 , i * sizeHeight + sizeHeight / 2, 0);
                        desenharRetanguloComTextura(sizeWidth, sizeHeight, texturasMundo.plataforma[l]);
                    glPopMatrix();
                }
                if(fase[linha_atual][j]==3){
                    glColor3f(1, 1, 1);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glPushMatrix();
                        glTranslatef(j * sizeWidth + sizeWidth / 2 , i * sizeHeight + sizeHeight / 2, 0);
                        desenharRetanguloComTextura(sizeWidth, sizeHeight, texturasMundo.blocoDeslizante);
                    glPopMatrix();
                }
            }
        }
        glEndList();
    }
    free(arquivo);
}

void trataColisao(){
    int dir = (personagem.x + personagem.width / 2) / sizeWidth;
    
    //colisao tela esquerda
    if(personagem.x - (personagem.width/2) <= 0){
        personagem.x=personagem.width/2;
    }
    //colisao tela direita
    if(personagem.x + (personagem.width/2) >= LARGURA_DO_MUNDO){
        personagem.x = LARGURA_DO_MUNDO - personagem.width/2;
    }
    
    //Caiu da fase
    if(personagem.y - personagem.height/2 <= camera.bottom){
        gameOver();
    }

    if(personagem.y + personagem.height/2 >= camera.top){
        personagem.pulo = 0;
    } 
}

void desabilitaPulo(){
    teclas[4]=0;
}

void atualizaAtaque(){
    personagem.podeAtacar=1;
}

void atualizaEscudo(){
    personagem.podeDefender=1;
}

int checarAtaque(double x, double y, double width, double height){
    double range = personagem.width +personagem.width/2;
    double xPersonagem = personagem.x;
    double xInimigo = 0;

    if(personagem.direcao == 1){
        xInimigo = x-width/2;
        if(
            (xInimigo > xPersonagem && xInimigo < xPersonagem + range) && 
            ((y-height/2 >= personagem.y - personagem.height/2 && y-height/2 < personagem.y + personagem.height/2) ||
            (y+height/2 >= personagem.y - personagem.height/2 && y+height/2 < personagem.y + personagem.height/2) ||
            ((xInimigo > xPersonagem && (xInimigo >= xPersonagem/2) &&
            (y+height/2 >= personagem.y - personagem.height/2 && y+height/2 < personagem.y + personagem.height/2))))
        ){
            return 1;
        }
    
    }

    if(personagem.direcao == -1){
        xInimigo = x+width/2;
        if(
            (xInimigo < xPersonagem && xInimigo > xPersonagem - range) && 
            ((y-height/2 >= personagem.y - personagem.height/2 && y-height/2 < personagem.y + personagem.height/2) ||
            (y+height/2 >= personagem.y - personagem.height/2 && y+height/2 < personagem.y + personagem.height/2) ||
            ((xInimigo < xPersonagem && xInimigo >= xPersonagem) && 
            ((y-height/2 >= personagem.y - personagem.height/2 && y-height/2 < personagem.y + personagem.height/2))))
        ){
            return 1;
        } 

    }
    if(abs(xInimigo-xPersonagem)<personagem.width/2 && 
        (y-height/2 >= personagem.y - personagem.height/2 && y-height/2 < personagem.y + personagem.height/2))
        return 1;
    

    return 0;
}

void trataAnimacaoTeleporteBoss(){
    boss.angulo+=30;
    boss.width -= 1;
    boss.height -= 1;

    if(boss.angulo < 360){
        glutTimerFunc(33, trataAnimacaoTeleporteBoss, 0);
    }
    else{
        int random = rand()%3;
        if(boss.posicao == random){
            if(boss.posicao < 2)
                boss.posicao++;
            else
                boss.posicao = 0;
        }else{
            boss.posicao = random;
        }

        if(boss.posicao == 0){
            boss.y = ((LINHAS_FASE - 29)*sizeHeight) + 9;
            boss.x = LARGURA_DO_MUNDO/2;
        }else if(boss.posicao == 1){
            boss.y = ((LINHAS_FASE - 21)*sizeHeight) + 9;
            boss.x = 29 * sizeWidth;
        }else{
            boss.y = ((LINHAS_FASE - 21)*sizeHeight) + 9;
            boss.x = 11 * sizeWidth;
        }
        boss.angulo = 0;
        boss.width = 10;
        boss.height = 10;
    }
}

void teleportarBoss(){
    boss.angulo = 0;
    trataAnimacaoTeleporteBoss();
}

void trataCorDoHitBoss(){
    boss.levouDano=0;
}

void trataAnimacaoBossFinal(){
    bossFinal.spriteY=0;
    if(bossFinal.spriteX>=.944){
        bossFinal.spriteX=0.03333;
    }
    else
        bossFinal.spriteX+=.03333;
    return;
}

void desenharBossFinal(){
    trataAnimacaoBossFinal();

    if(bossFinal.levouDano){
        glColor3f(1,0,0);
    }
    else
        glColor3f(1,1,1);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, bossFinal.textura);
    glPushMatrix();
    glTranslatef(bossFinal.x, bossFinal.y, .85);

        glBegin(GL_QUADS);
        glTexCoord2f(bossFinal.spriteX,bossFinal.spriteY); 
            glVertex2f(-bossFinal.width/2 , -bossFinal.height/2);
        glTexCoord2f(bossFinal.spriteX+.033,bossFinal.spriteY);
            glVertex2f(bossFinal.width/2, -bossFinal.height/2);
        glTexCoord2f(bossFinal.spriteX+.033,bossFinal.spriteY+1);
            glVertex2f(bossFinal.width/2, bossFinal.height/2);
        glTexCoord2f(bossFinal.spriteX,bossFinal.spriteY+1);
            glVertex2f(-bossFinal.width/2, bossFinal.height/2);
        glEnd();

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void atualizarBossFinal(){
    if(bossFinal.ativo && bossFinal.existe){
        double base = 1;
        double yBoss = boss.y + boss.height/2;
        double posProjetilX;
        double posProjetilY;
        int direcaoProjetilX;
        int direcaoProjetilY;

        int tempo = time(0);

        if(bossFinal.travado && !bossFinal.espiral){
            double distanciaX = LARGURA_DO_MUNDO/2 - bossFinal.x;
            double distanciaY = camera.bottom + 40 - bossFinal.y;

            double norma = sqrt(distanciaX*distanciaX + distanciaY*distanciaY);
            bossFinal.x += distanciaX/norma; 
            bossFinal.y += distanciaY/norma; 

            if(fabs(distanciaX) < 1  && fabs(distanciaY) < 1){
                bossFinal.espiral = 1;
                bossFinal.timerEspiral = time(0);
            }
        }

        if(bossFinal.espiral){
            double x = bossFinal.x + bossFinal.raio * cos(bossFinal.angulo);
            double y = bossFinal.y + bossFinal.raio * sin(bossFinal.angulo);
            criaProjetilCircular(x, y, 0, 0, 10, bossFinal.raio, bossFinal.angulo, bossFinal.x, bossFinal.y);
            bossFinal.angulo += 2 * M_PI/40;
            bossFinal.raio += 0.2; 

            if(bossFinal.raio >= 50){
                bossFinal.raio = 0;
            }
            
            if(bossFinal.angulo >= 2*M_PI)
                bossFinal.angulo -= 2* M_PI;

            if(tempo - bossFinal.timerEspiral >= 5){
                bossFinal.travado = 0;
                bossFinal.espiral = 0;
            }
        }

        if(!bossFinal.travado && tempo - bossFinal.timerMovimento >= 2){
            double distanciaY;
            double distanciaX;

            if(bossFinal.velX > 0){
                distanciaX = LARGURA_DO_MUNDO - (bossFinal.x + bossFinal.width/2);
                posProjetilX = bossFinal.x - bossFinal.width/2;
                posProjetilY = bossFinal.y;
                direcaoProjetilX = -1;
            }else if(bossFinal.velX < 0){
                distanciaX = (bossFinal.x - bossFinal.width/2);
                posProjetilX = bossFinal.x + bossFinal.width/2;
                posProjetilY = bossFinal.y;
                direcaoProjetilX = 1;
            }
            
            if(bossFinal.velY > 0){
                distanciaY = LINHAS_FASE*sizeHeight - (bossFinal.y + bossFinal.height/2);
                posProjetilX = bossFinal.x;
                posProjetilY = bossFinal.y - bossFinal.height/2;
                direcaoProjetilY = -1;
            }else if(bossFinal.velY < 0){
                distanciaY = (bossFinal.y - bossFinal.height/2) - ((LINHAS_FASE-19)*sizeHeight);
                posProjetilX = bossFinal.x;
                posProjetilY = bossFinal.x + bossFinal.height/2;
                direcaoProjetilY = 1;
            }
            
            if(distanciaX < fabs(bossFinal.velX)){
                bossFinal.x += distanciaX;
                bossFinal.velX *= base *-1;
                bossFinal.timerMovimento = tempo;
            }else {
                bossFinal.x += bossFinal.velX;
            }

            if(distanciaY < fabs (bossFinal.velY)){
                bossFinal.y += distanciaY;
                bossFinal.velY *= base *-1;
                bossFinal.timerMovimento = tempo;
            }else {
                bossFinal.y += bossFinal.velY;
            }

        }
        
        if((bossFinal.y - bossFinal.height/2) > (LINHAS_FASE-19)*sizeHeight && (bossFinal.y + bossFinal.height/2) < LINHAS_FASE*sizeHeight){
            if((tempo - bossFinal.timerAtaque1) >= 1){
                criaProjetil(bossFinal.x + bossFinal.width/2, bossFinal.y + bossFinal.height/2, 1, 1, 1);
                criaProjetil(bossFinal.x + bossFinal.width/2, bossFinal.y - bossFinal.height/2, 1,-1, 1);
                criaProjetil(bossFinal.x - bossFinal.width/2, bossFinal.y - bossFinal.height/2, -1, -1, 1);
                criaProjetil(bossFinal.x - bossFinal.width/2, bossFinal.y + bossFinal.height/2, -1, 1, 1);
                criaProjetil(bossFinal.x + bossFinal.width/2, bossFinal.y, 1, 0, 1);
                criaProjetil(bossFinal.x - bossFinal.width/2, bossFinal.y, -1, 0, 1);
                criaProjetil(bossFinal.x, bossFinal.y + bossFinal.height/2, 0, 1, 1);
                criaProjetil(bossFinal.x, bossFinal.y - bossFinal.height/2, 0, -1, 1);
                bossFinal.timerAtaque1 = tempo;
            }
        }
    }
}

void trataCorDoHitBossFinal(){
    bossFinal.levouDano=0;
}

void desenhaAtaque(){
    if(!personagem.travado){
        if(mouse[0] && personagem.podeAtacar){
            for(int i = 0; i < quantidade_inimigos; i++){
                if(!inimigos[i].morreu && checarAtaque(inimigos[i].x, inimigos[i].y, inimigos[i].width, inimigos[i].height)){
                    inimigos[i].vidas --;
                    inimigos[i].levouDano=1;
                    if(inimigos[i].vidas == 0){
                        inimigos[i].spriteX=0;
                        //inimigos[i].morreu = 1;
                        game.pontuacao_atual += 20;
                    }
                }
            }
            if(boss.existe){
                if(fase_atual == 3 && boss.ativo){
                    if(checarAtaque(boss.x, boss.y, boss.width+2*sizeWidth, boss.height+2)){
                        boss.vida -= 1;
                        boss.levouDano=1;
                        glutTimerFunc(100,trataCorDoHitBoss,0);

                        teleportarBoss();
                        if(boss.vida == 0){
                            game.pontuacao_atual += 500;
                            boss.existe = 0;
                            portal.existe=1;

                            if(game.pontuacao_atual > game.maior_pontuacao){
                                game.maior_pontuacao = game.pontuacao_atual;
                                game.pontuacao_atual = 0;
                            }
                        }
                    }
                }
            }
            if(bossFinal.existe){
                if(fase_atual == 5 && bossFinal.ativo){
                    if(checarAtaque(bossFinal.x, bossFinal.y, bossFinal.width+2*sizeWidth, bossFinal.height-10)){
                        bossFinal.vida -= 1;
                        bossFinal.levouDano=1;
                        glutTimerFunc(100,trataCorDoHitBossFinal,0);
                        if(bossFinal.vida % 4 == 0 && !bossFinal.espiral){
                            bossFinal.travado = 1;
                        }

                        if(bossFinal.vida == 0){
                            game.pontuacao_atual += 1000;
                            bossFinal.existe = 0;
                            portal.existe=1;

                            if(game.pontuacao_atual > game.maior_pontuacao){
                                game.maior_pontuacao = game.pontuacao_atual;
                                game.pontuacao_atual = 0;
                            }
                        }
                    }
                }
            }
            personagem.podeAtacar=0;
            glutTimerFunc(400, atualizaAtaque, 0);
        }
    }
    
}

void pararEscudo(){
    personagem.estaDefendendo = 0;
    personagem.podeDefender = 0;
}

void criaEscudo(){
    if(mouse[1] && personagem.podeDefender){
        personagem.movX=0;
        personagem.estaDefendendo = 1;
        glutTimerFunc(2000, atualizaEscudo, 0);
        glutTimerFunc(500, pararEscudo, 0);
    }
}

void trataPulo(){

    if(personagem.y>personagem.maxPulo-15 && (fase[LINHAS_FASE-1-personagem.i][personagem.j]||fase[LINHAS_FASE-1-personagem.i][personagem.j+1])){
        glutTimerFunc(33,desabilitaPulo,0);
    } 
    
    if(fase[LINHAS_FASE-personagem.i][personagem.j] && teclas[4]){
        personagem.maxPulo = personagem.y + 4*sizeHeight;
        personagem.pulo=1;
    }
    
    if(fase[LINHAS_FASE-personagem.i][personagem.j+2] && teclas[4] && personagem.j<COLUNAS_FASE-2){
        personagem.maxPulo = personagem.y + 4*sizeHeight;
        personagem.pulo=1;
    }

    if(!personagem.travado){

        if(personagem.y<personagem.maxPulo && personagem.pulo){
            personagem.y+=sizeHeight/2;
        }

        if(personagem.y>=personagem.maxPulo){
            personagem.pulo=0;
            personagem.maxPulo=0;   
        }  

        if(!personagem.pulo){
            personagem.y-=sizeHeight/2;
        }
    }

    //colisao chão
    if(personagem.y-personagem.height/2<=sizeHeight*2){
        personagem.y =sizeHeight*2+personagem.height/2;
    }
    
    if(fase[LINHAS_FASE-personagem.i][personagem.j]||fase[LINHAS_FASE-personagem.i][personagem.j+2]){
        if(!teclas[4]){
            personagem.y = personagem.i*sizeHeight+personagem.height/2;
            personagem.maxPulo=0;
        }
    }

    if(personagem.j>=37&&(fase[LINHAS_FASE-personagem.i][personagem.j+2]||fase[LINHAS_FASE-personagem.i][personagem.j+2])){
        if(!teclas[4]){
            personagem.y = personagem.i*sizeHeight+personagem.height/2;
            personagem.maxPulo=0;
        }
    }

    if(fase[LINHAS_FASE-personagem.i][personagem.j+1]){
        if(!teclas[4]){
            personagem.y = personagem.i*sizeHeight+personagem.height/2;
            personagem.maxPulo=0;
        }
    }
    
}

void desenhaVidas(){
    GLuint textura = personagem.idPowerUp==1 ? texturasMundo.vidasPowerUp : texturasMundo.vida1;
    for(int i = 0; i < MAX_VIDAS; i++){
        glColor4f(1, 1, 1, 1);

        if(i >= personagem.vidas){
            textura = texturasMundo.vida2;
        }
        glPushMatrix();
            glTranslatef(120 + i*10, camera.bottom+20, 0.81);
            desenharRetanguloComTextura(10, 10, textura);
        glPopMatrix();
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void trataAnimacaoDoPersonagem(){
    if(personagem.estaDefendendo){
        personagem.spriteX=0;
        personagem.spriteY=0;
        return;
    }

    if(mouse[0]&&personagem.podeAtacar && !personagem.travado){
        personagem.spriteX=.033;
        personagem.spriteY=0;
        return;
    }

    if(!personagem.movX && !personagem.movY){
        personagem.spriteY=.3333;
        if(personagem.spriteX>=.944){
            personagem.spriteX=0.03333;
        }
        else
            personagem.spriteX+=.03333;
        return;
    }
    if(personagem.movX && ! personagem.movY){
        personagem.spriteY=.6666;
        if(personagem.spriteX>=.944){
            personagem.spriteX=0.03333;
        }
        else
            personagem.spriteX+=.03333;
        return;
    }

}

void desenhaPersonagem(){
    trataAnimacaoDoPersonagem();
    // Desenhar personagem
    glPushMatrix();
    glTranslatef(personagem.x, personagem.y, 0.5);
    if(!personagem.rotacao){
        personagem.fatorEscalaX=1;
        personagem.fatorEscalaY=1;
    }
    if(personagem.rotacao && personagem.fatorEscalaX>0){
        personagem.travado=1;
        personagem.fatorEscalaX-=.05;
        personagem.fatorEscalaY-=.05;
        glScalef(personagem.fatorEscalaX,personagem.fatorEscalaY,0);
        glRotatef(20+personagem.rotacao,0,0,1);
    }
    
    if(!personagem.travado){
        if(personagem.direcao<0){
            glRotatef(180,0,1,0);
        }   
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, personagem.textura);

    if(personagem.levouDano){
        glColor3f(1,0,0);
    }
    else{
        if(personagem.idPowerUp==1){
            glColor3f(1,1,0);
        }
        else{
            glColor3f(1, 1, 1);
        }
    }
        

    glBegin(GL_POLYGON);
        glTexCoord2f(personagem.spriteX,personagem.spriteY); 
        glVertex2f(-personagem.width / 2, -personagem.height / 2);
        glTexCoord2f(personagem.spriteX+.033,personagem.spriteY);
        glVertex2f(personagem.width / 2, -personagem.height / 2);
        glTexCoord2f(personagem.spriteX+.033,personagem.spriteY+.33);
        glVertex2f(personagem.width / 2, personagem.height / 2);
        glTexCoord2f(personagem.spriteX,personagem.spriteY+.33);
        glVertex2f(-personagem.width / 2, personagem.height / 2);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    glPopMatrix();
}

void atualizaParallaxFase1(){
    glColor4f(1,1,1,1);

    //sol
    glPushMatrix();
    if(personagem.y>=400)
        glTranslatef(LARGURA_DO_MUNDO/5, 400, -.3);
    else{
        if(personagem.y>=200)
            glTranslatef(LARGURA_DO_MUNDO/5, camera.bottom+ALTURA_DO_MUNDO/2+(time(0)%5), -.3);
        else
            glTranslatef(LARGURA_DO_MUNDO/5, 200, -.3);
    }
        
    desenharRetanguloComTextura(20,20,texturasMundo.fundo1[0]);
    glPopMatrix();

    //ilhas e mar
    //glColor4f(1,1,1,.7);
    glPushMatrix();
    if(personagem.y>=170)
        glTranslatef(LARGURA_DO_MUNDO/2, 170, -.2);
    else{
        if(personagem.y>=100)
            glTranslatef(LARGURA_DO_MUNDO/2, camera.bottom+ALTURA_DO_MUNDO/2+(time(0)%3), -.2);
        else
            glTranslatef(LARGURA_DO_MUNDO/2, 100, -.2);
    }
        
    desenharRetanguloComTextura(LARGURA_DO_MUNDO,200,texturasMundo.fundo2[0]);
    glPopMatrix();
}

void atualizaParallaxFase2(){
    glColor4f(1,1,1,.7);
    glPushMatrix();
    if(personagem.y>=500)
        glTranslatef(LARGURA_DO_MUNDO/2, 500, -.3);
    else{
        if(personagem.y>=250)
            glTranslatef((LARGURA_DO_MUNDO/2), camera.bottom+ALTURA_DO_MUNDO/2+(time(0)%5), -.3);
        else
            glTranslatef(LARGURA_DO_MUNDO/2, 280, -.3);
    }
    glRotatef((int)personagem.x%360,0,0,1);
    desenharRetanguloComTextura(80,100,texturasMundo.fundo2[1]);
    glPopMatrix();

    glColor4f(1,1,1,1);
    glPushMatrix();
    
    if(personagem.y>=170)
        glTranslatef(LARGURA_DO_MUNDO/2, 170, -.2);
    else{
        if(personagem.y>=100)
            glTranslatef(LARGURA_DO_MUNDO/2, camera.bottom+ALTURA_DO_MUNDO/2+(time(0)%3), -.2);
        else
            glTranslatef(LARGURA_DO_MUNDO/2, 100, -.2);
    }
    
    desenharRetanguloComTextura(LARGURA_DO_MUNDO,200,texturasMundo.fundo1[1]);
    glPopMatrix();
}

void atualizaParallaxFase3(){
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(0, camera.bottom, .1);
    
    glBindTexture(GL_TEXTURE_2D, texturasMundo.fundo1[2]);

    glBegin(GL_POLYGON);
        glTexCoord2f(0,0); glVertex2f(0,0);
        glTexCoord2f(1,0); glVertex2f(LARGURA_DO_MUNDO,0);
        glTexCoord2f(1,1); glVertex2f(LARGURA_DO_MUNDO,ALTURA_DO_MUNDO);
        glTexCoord2f(0,1); glVertex2f(0, ALTURA_DO_MUNDO);
    glEnd();

    glPopMatrix();

}

void atualizaParallaxFase4(){
    glColor4f(1,1,1,1);

    //sol
    glPushMatrix();
    if(personagem.y>=400)
        glTranslatef(LARGURA_DO_MUNDO/5, 400, -.3);
    else{
        if(personagem.y>=200)
            glTranslatef(LARGURA_DO_MUNDO/5, camera.bottom+ALTURA_DO_MUNDO/2+(time(0)%5), -.3);
        else
            glTranslatef(LARGURA_DO_MUNDO/5, 200, -.3);
    }
        
    desenharRetanguloComTextura(20,20,texturasMundo.fundo2[3]);
    glPopMatrix();

    //ilhas e mar
    //glColor4f(1,1,1,.7);
    glPushMatrix();
    if(personagem.y>=170)
        glTranslatef(LARGURA_DO_MUNDO/2, 170, -.2);
    else{
        if(personagem.y>=100)
            glTranslatef(LARGURA_DO_MUNDO/2, camera.bottom+ALTURA_DO_MUNDO/2+(time(0)%3), -.2);
        else
            glTranslatef(LARGURA_DO_MUNDO/2, 100, -.2);
    }
        
    desenharRetanguloComTextura(LARGURA_DO_MUNDO,250,texturasMundo.fundo1[3]);
    glPopMatrix();
}

void atualizaParallaxFase5(){
    glColor4f(1,1,1,.9);
        
    desenharRetanguloComTextura(20,20,texturasMundo.fundo2[4]);
    glPopMatrix();

    //ilhas e mar
    //glColor4f(1,1,1,.7);
    glPushMatrix();
    if(personagem.y>=170)
        glTranslatef(LARGURA_DO_MUNDO/2, 170, -.2);
    else{
        if(personagem.y>=100)
            glTranslatef(LARGURA_DO_MUNDO/2, camera.bottom+ALTURA_DO_MUNDO/2+(time(0)%3), -.2);
        else
            glTranslatef(LARGURA_DO_MUNDO/2, 100, -.2);
    }
        
    desenharRetanguloComTextura(LARGURA_DO_MUNDO,200,texturasMundo.fundo1[4]);
    glPopMatrix();
}

void desenhaChao(){
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturasMundo.grama[fase_atual-1]);
    glBegin(GL_POLYGON);
        glTexCoord2f(0,0);
        glVertex2f(0,0);
        glTexCoord2f(1,0);
        glVertex2f(LARGURA_DO_MUNDO,0);
        glTexCoord2f(1,1);
        glVertex2f(LARGURA_DO_MUNDO,sizeHeight*2);
        glTexCoord2f(0,1);
        glVertex2f(0, sizeHeight*2);
    glEnd();
}

void desenhaPontuacao(){
    glColor3f(1, 1, 1);
    char *texto = malloc(30*sizeof(char));
    sprintf(texto, "%d Pontos", game.pontuacao_atual);
    escreveTexto(GLUT_BITMAP_8_BY_13, texto, LARGURA_DO_MUNDO - 40, camera.top-10, 0.8);
    free(texto);
}

void trataAnimacaoPortal(){
    if(portal.spriteX>=.944){
        portal.spriteX=0;
    }
    else
        portal.spriteX+=.03333;
    return;
}

void desenhaPortal(){
    trataAnimacaoPortal();
    glColor3f(1, 1, 1);
    glPushMatrix();
    glTranslatef(portal.x, portal.y, .4);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, portal.textura);
    glBegin(GL_POLYGON);
    glTexCoord2f(portal.spriteX,1);
        glVertex3f(-portal.width / 2, portal.height / 2, 0);
    glTexCoord2f(portal.spriteX+0.03333,1);
        glVertex3f(portal.width / 2, portal.height / 2, 0);
    glTexCoord2f(portal.spriteX+0.03333,0);
        glVertex3f(portal.width / 2 , -portal.height / 2, 0);
    glTexCoord2f(portal.spriteX,0);
        glVertex3f(-portal.width / 2 -.2, -portal.height / 2 -.2, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

}

void encerraPowerUpArmadura(){
    personagem.idPowerUp=0;
}

void trataCapturaDePowerUps(PowerUp* poder){
    if(abs(personagem.x-poder->x)<=poder->width/2){
        if(abs(personagem.y-poder->y)<=poder->height/2){
            if(poder->id == 1){
            poder->existe=0;
            personagem.idPowerUp=poder->id;
            personagem.vidas= MAX_VIDAS;
            if(poder->id==1)
                glutTimerFunc(10000,encerraPowerUpArmadura,0);
        }
        
        if(poder->id == 2){
            if(personagem.vidas != MAX_VIDAS){
            personagem.vidas++;
             poder->existe=0;
            }
        
            }
        }
    }
}

void desenharPowerUps(){
    for(int i=0; i<50; i++){
        if(powerUps[i].existe){
            
            glPushMatrix();
            trataCapturaDePowerUps(&powerUps[i]);
            glTranslatef(powerUps[i].x, powerUps[i].y + time(0)%2,.6);
            glColor3f(1,1,1);

            if(powerUps[i].id==1){              
                glColor3f(1,1,1);
                desenharRetanguloComTextura(powerUps->width, powerUps->height, texturasMundo.powerUpArmadura);
            }
             if(powerUps[i].id==2){
                desenharRetanguloComTextura(powerUps->width, powerUps->height, texturasMundo.vida1);                                
            }
            
            
            glPopMatrix();
        }
    }
}

void desenhaTransicaoPortal(){
    glColor4f(0,0,0,personagem.rotacao/100);
    glPushMatrix();
    glTranslatef(LARGURA_DO_MUNDO/2, 300,1);
    desenharRetangulo(LARGURA_DO_MUNDO, 600);
    glPopMatrix();
}

void desenharFase()
{
    // Desenhar cenário
    glCallList(listas_fases[fase_atual-1]);

    if(fase_atual==1){
        atualizaParallaxFase1();
    }
    if(fase_atual==2){
        atualizaParallaxFase2();
    }
    if(fase_atual==3){
        atualizaParallaxFase3();
    }
    if(fase_atual==4){
        atualizaParallaxFase4();
    }

    if(fase_atual==5){
        atualizaParallaxFase5();
    }

    if(portal.existe)
        desenhaPortal();
    
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    // Desenhar personagem
    
    desenhaPersonagem();

    desenharPowerUps();

    desenharInimigos();
    desenhaProjetil();
    desenhaVidas();

    desenhaAtaque();
    criaEscudo();

    if(fase_atual==3 && boss.existe)
        desenharBoss();

    if(fase_atual == 5 && bossFinal.existe)
        desenharBossFinal();

    desenhaPontuacao();

    if(gameover.comecou && !gameover.acabou){
        desenhaGameOver();
    }

    if(creditos.comecou && !creditos.acabou){
        desenhaCreditos();
    }

    if(personagem.rotacao && fase_atual != 5){
        desenhaTransicaoPortal();
    }

    desenharObstaculos();
}

void desenharControles()
{
    glColor3f(1,1,1);
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturasMundo.telaControles);
    glTranslatef(LARGURA_DO_MUNDO / 2, ALTURA_DO_MUNDO / 2, 0);
    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0,1);
        glVertex3f(-LARGURA_DO_MUNDO / 2, ALTURA_DO_MUNDO / 2, 0);
    glTexCoord2f(1,1);
        glVertex3f(LARGURA_DO_MUNDO / 2, ALTURA_DO_MUNDO / 2, 0);
    glTexCoord2f(1,0);
        glVertex3f(LARGURA_DO_MUNDO / 2, -ALTURA_DO_MUNDO / 2, 0);
    glTexCoord2f(0,0);
        glVertex3f(-LARGURA_DO_MUNDO / 2, -ALTURA_DO_MUNDO / 2, 0);
    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);

    for (int i = 0; i < quantidade_botoes; i++)
    {
        Button btn = *buttons[i];
        if (btn.id / 10 == tela_atual)
        {
            glColor3f(btn.cores[0], btn.cores[1], btn.cores[2]);
            glPushMatrix();
            glTranslatef(btn.x, btn.y, 0.5);
            glColor3f(1,1,1);
            if(btn.id==10)
                desenharRetanguloComTextura(btn.width, btn.height, texturasMundo.botaoVoltar);
            glPopMatrix();
        }
    }
}

void desenhaCena()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    switch (tela_atual)
    {
    case 0:
        desenharMenu();
        break;
    case 1:
        desenharControles();
        break;
    case 2:
        desenharFase();
        break;
    }
    
    glutSwapBuffers();
}

void inicializaPersonagem(){
    personagem.textura = carregaTextura("imagens/personagem.png");
    personagem.podeAtacar=1;
    personagem.vidas=MAX_VIDAS;
    personagem.spriteX=personagem.spriteY=0;
    personagem.fatorEscalaX=1;
    personagem.fatorEscalaY=1;
}

void inicializaPowerUps(){
    for(int i=0; i<50; i++){
        powerUps[i].existe=0;
        powerUps[i].width=powerUps[i].height=10;
        powerUps[i].id=0;
    }
}

void inicializa()
{
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    inicializaTexturas();
    inicializaPersonagem();
    inicializarInimigos();
    inicializaProjetil();
    criaListasFases();
    inicializaPowerUps();

    glClearColor(0, 0, 0, 1.0);
    buttons[0] = &btnStart;
    buttons[1] = &btnControls;
    buttons[2] = &btnVoltar;
    camera.x = 0;
    camera.y = 0;
    camera.bottom = 0;
    camera.top = ALTURA_DO_MUNDO;
    camera.barraHorizontal = 0;
    camera.barraVertical = 0;
    camera.arena = 0;
    portal.height=2*sizeHeight;
    portal.width=2*sizeWidth;
    
}

void movimentouMouse(int x, int y)
{
    double xMundo, yMundo;
    transformarCoordenadas(&xMundo, &yMundo, x, y);

    if (tela_atual != 2)
    {
        int resultado = hoverMenu(tela_atual, xMundo, yMundo, buttons, quantidade_botoes);
        if (resultado == 1)
            glutPostRedisplay();
    }
}

void teclaPressionada(unsigned char key, int x, int y)
{
    switch (tolower(key))
    {
    case 'w':
        teclas[0] = 1;
        break;
    case 'a':
        teclas[1] = 1;
        break;
    case 's':
        teclas[2] = 1;
        break;
    case 'd':
        teclas[3] = 1;
        break;
    case 27:
        exit(0);
    break;
    
    case 32:
        teclas[4]=1;
    break;

    default:
        break;
    }
}

void teclaLiberada(unsigned char key, int x, int y)
{
    switch (tolower(key))
    {
    case 'w':
        teclas[0] = 0;
        personagem.movY = 0;
        break;
    case 'a':
        teclas[1] = 0;
        personagem.movX = 0;
        break;
    case 's':
        teclas[2] = 0;
        personagem.movY = 0;
        break;
    case 'd':
        teclas[3] = 0;
        personagem.movX = 0;
        break;
    case 32:
        teclas[4]=0;
    break;
    }
}

void posichao()
{
    personagem.j = (personagem.x - personagem.width / 2) / sizeWidth;
    personagem.i = (personagem.y - personagem.height / 2) / sizeHeight;
    //printf("COORDENADAS MUNDO: %.1f %.1f\n", personagem.x, personagem.y);
    //printf("COORDENADAS MATRIX: %i %i\n", LINHAS_FASE-personagem.i, personagem.j);
}

void atualizarRounds(){
    for(int i = 0; i < 3; i++){
        if(rounds[i].comecou){
            int tempo = time(0);
            if(tempo - rounds[i].timer > rounds[i].tempoDeRecarga && rounds[i].totalInimigos > 0){
                double width = 5, height = 8;
                int velX = rand() % 2 == 0 ? -1 : 1;
                int tipo = rounds[i].tipos[rand() % rounds[i].total_tipos];
                int pos = rand() % rounds[i].total_posicoes;
                int x = rounds[i].posicoesX[pos] * sizeWidth;
                int y = (LINHAS_FASE - rounds[i].posicoesY[pos]) * sizeHeight + height/2;

                if(tipo == 4)
                    x = velX > 0 ? 0 : LARGURA_DO_MUNDO;

                criarInimigo(x, y, width, height, velX, 0, 2, tipo);

                rounds[i].timer = tempo;
                rounds[i].totalInimigos --;
            } 
            int acabou = 0;
            if(rounds[i].totalInimigos == 0){
                acabou = 1;
                for(int i = 0; i < quantidade_inimigos; i++){
                    if(!inimigos[i].morreu){
                        if(inimigos[i].y > camera.bottom && inimigos[i].y < camera.top){
                            acabou = 0;
                        }
                    }
                }
            }
            rounds[i].comecou = !acabou;
            if(rounds[i].comecou == 0){
                //printf("Acabou o round %d!\n", i+1);
                camera.travada = 0;
            }
        }
    }
}

void deslizarPersonagem(){
    if(fase_atual == 4){
        if(
            fase[LINHAS_FASE - personagem.i][personagem.j] == 3 ||
            fase[LINHAS_FASE - personagem.i][personagem.j+1] == 3 ||
            fase[LINHAS_FASE - personagem.i][personagem.j+2] == 3
        ){
            personagem.movX = 0.3 * personagem.direcao;
        }else if(teclas[1] == 0 && teclas[3] == 0){
            personagem.movX = 0;
        }
    }
}

void movimentarPersonagem(){
    if(!personagem.travado){
        if (teclas[1])
        {
            if(personagem.movX > -2){
                personagem.movX -= 2 + personagem.movX;
            }
            personagem.direcao = -1;
        }
            
        if (teclas[3])
        {
            if(personagem.movX < 2){
                personagem.movX += (2 - personagem.movX);
            }
            personagem.direcao = 1;
        }
        personagem.x += personagem.movX;
        personagem.y += personagem.movY;
    }
}

void atualizaCena()
{
    if(tela_atual == 2){
        movimentarPersonagem();
        posichao();
        trataPulo();
        trataColisao();
        deslizarPersonagem();
   
        atualizarInimigos();
        atualizaProjetil();

        if(fase_atual==3){
            if(personagem.y >= (LINHAS_FASE-29) * sizeHeight){
                camera.arena = 1;
                camera.travada = 1;
            }
            atualizarBoss();
        }

        if(fase_atual == 5){
            if(personagem.y >= (LINHAS_FASE-19) * sizeHeight){
                camera.arena = 1;
                camera.travada = 1;
            }
            atualizarBossFinal();
        }
        
        atualizarObstaculos();
        atualizarRounds();

        for(int i = 0; i < 3; i++){
            if(rounds[i].fase == fase_atual && personagem.y >= rounds[i].alturaMinima && rounds[i].totalInimigos > 0 && !rounds[i].comecou){
                rounds[i].timer = time(0);
                rounds[i].comecou = 1;
                camera.y = rounds[i].alturaMinima + ALTURA_DO_MUNDO/2 - 20;
                camera.travada = 1;
            }
        }
        if(portal.existe){
            if(abs(personagem.x-portal.x)<=portal.width/2 && abs(personagem.y-portal.y)<=portal.height/2){
                if(fase_atual == 5){
                    subirCreditos();
                    portal.existe=0;
                }
                else{
                    personagem.movX=0;
                    personagem.rotacao+=10;

                    if(personagem.fatorEscalaX<=0 && fase_atual!=5){
                        personagem.travado=0;
                        fase_atual++;
                        inicializarFase();
                    }
                }   
        }
        else
            personagem.rotacao=0;
        }
        glutTimerFunc(33, atualizaCena, 0);
    }
    atualizaCamera();
    glutPostRedisplay();
}

void clicouNaTela(int button, int state, int x, int y)
{
    double xMundo, yMundo;
    transformarCoordenadas(&xMundo, &yMundo, x, y);
    if (tela_atual != 2)
    {
        tela_atual = cliqueMenu(tela_atual, xMundo, yMundo, buttons, quantidade_botoes, button, state);
        if(tela_atual == 2){
            glutTimerFunc(33, atualizaCena, 0);
            inicializarFase(fase_atual);
        }
    }else if(tela_atual == 2){
        if(button == GLUT_LEFT_BUTTON){
            if(state==GLUT_DOWN){
                mouse[0]=1;
            }
            else{
                mouse[0]=0;
            }
        }else{
            if(state==GLUT_DOWN){
                mouse[1]=1;
                
            }
            else{
                mouse[1]=0;
            }
        }
    }
    glutPostRedisplay();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    srand(time(0));
    glutInitContextVersion(1, 1);
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(640, 480);
    glutInitWindowPosition(200, 100);
    glutCreateWindow("Dangerous Heights");

    inicializa();

    glutDisplayFunc(desenhaCena);
    glutReshapeFunc(redimensionaCena);
    glutMouseFunc(clicouNaTela);
    glutPassiveMotionFunc(movimentouMouse);
    glutKeyboardFunc(teclaPressionada);
    glutKeyboardUpFunc(teclaLiberada);

    glutMainLoop();

    return 0;
}
