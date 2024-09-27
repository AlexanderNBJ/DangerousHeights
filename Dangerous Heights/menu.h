#ifndef MENU_H
#define MENU_H
#endif

typedef struct{
    double x;
    double y;
    double width;
    double height;
    double cores[3];
    int id;
    int hover;
}Button;


void hoverStart();
void fimHoverStart();
void hoverControls();
void fimHoverControls();
void hoverVoltar();
void fimHoverVoltar();
int cliqueMenu(int tela_atual, double xMundo, double yMundo, Button* buttons[], int quantidade_botoes, int button, int state);
int hoverMenu(int tela_atual, double xMundo, double yMundo, Button* buttons[], int quantidade_botoes);
