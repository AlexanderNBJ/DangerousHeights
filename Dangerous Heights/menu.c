#include "menu.h"

void hoverStart(Button *btn){
    btn->cores[0] = 0.8;
    btn->cores[1] = 0.1;
    btn->cores[2] = 0.1;
}

void fimHoverStart(Button *btn){
    btn->cores[0] = 0.6;
    btn->cores[1] = 0.1;
    btn->cores[2] = 0.1;
}

void hoverControls(Button *btn){
    btn->cores[0] = 0.1;
    btn->cores[1] = 0.8;
    btn->cores[2] = 0.1;
}

void fimHoverControls(Button *btn){
    btn->cores[0] = 0.1;
    btn->cores[1] = 0.6;
    btn->cores[2] = 0.1; 
}

void hoverVoltar(Button *btn){
    btn->cores[0] = 0.1;
    btn->cores[1] = 0.1;
    btn->cores[2] = 0.8;
}

void fimHoverVoltar(Button *btn){
    btn->cores[0] = 0.1;
    btn->cores[1] = 0.1;
    btn->cores[2] = 0.6; 
}

int cliqueMenu(int tela_atual, double xMundo, double yMundo, Button* buttons[], int quantidade_botoes, int button, int state){
    switch(button){
        case 0:
            if(state == 0){
                for(int i = 0; i < quantidade_botoes; i++){
                    Button btn = *buttons[i];
                    if(btn.id/10 == tela_atual){
                       if(
                            xMundo > btn.x-btn.width/2 && xMundo < btn.x+btn.width/2 &&
                            yMundo > btn.y-btn.height/2 && yMundo < btn.y+btn.height/2
                        ){
                            if(btn.id == 1){
                                tela_atual = 2;
                                break;
                            }else if(btn.id == 2){
                                tela_atual = 1;
                                break;
                            }else if(btn.id == 10){
                                tela_atual = 0;
                                break;
                            }
                        }
                    }
                }
            }
        break;
    }
    return tela_atual;
}

int hoverMenu(int tela_atual, double xMundo, double yMundo, Button* buttons[], int quantidade_botoes){
    for(int i = 0; i < quantidade_botoes; i++){
            Button btn = *buttons[i];
            if(btn.id/10 == tela_atual){
                if(
                    xMundo > btn.x-btn.width/2 && xMundo < btn.x+btn.width/2 &&
                    yMundo > btn.y-btn.height/2 && yMundo < btn.y+btn.height/2
                ){
                    if(btn.hover == 0){
                        buttons[i]->width += 1;
                        buttons[i]->height += 1;
                        if(btn.id == 1){
                            hoverStart(buttons[i]);
                        }else if(btn.id == 2){
                            hoverControls(buttons[i]);
                        }else if(btn.id == 10){
                            hoverVoltar(buttons[i]);
                        }
                        buttons[i]->hover = 1;
                        return 1;
                    }
                }
                else{
                    if(btn.hover == 1){
                        buttons[i]->width -= 1;
                        buttons[i]->height -= 1;
                        if(btn.id == 1){
                            fimHoverStart(buttons[i]);
                        }else if(btn.id == 2){
                            fimHoverControls(buttons[i]);
                        }else if(btn.id == 10){
                            fimHoverVoltar(buttons[i]);
                        }
                        buttons[i]->hover = 0;
                        return 1;
                    }
                }
            }
        }
    return 0;
}

