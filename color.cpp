#include <iostream>
#include <vector>
#include <math.h>
#include "CImg.h"
using namespace std;
using namespace cimg_library;

class Color{
    public:
        int red;
        int green;
        int blue;
    public:
        Color(){
            red = 0;
            green = 0;
            blue = 0;
        }
        Color(int r,int g, int b){
            red = r;
            green = g;
            blue = b;
        }
};

vector<int> get_position(Color color){
    int c,r;
    int i=7;
    vector<int> red_b(8,0);
	vector<int> green_b(8,0);
	vector<int> blue_b(8,0);
    c=color.red;
    while(c!=0){r=c%2;red_b[i]=r;c/=2;i--;}
    i=7;
    c=color.green;
    while(c!=0){r=c%2;green_b[i]=r;c/=2;i--;}
    i=7;
    c=color.blue;
    while(c!=0){r=c%2;blue_b[i]=r;c/=2;i--;}
    vector<int> pos;
    for(int j=0;j<8;j++){
        vector<int> temp;
        int decimal=0;
        for(int i=0;i<3;i++){
            temp.push_back(red_b[j]);
            temp.push_back(green_b[j]);
            temp.push_back(blue_b[j]);
            decimal = decimal << 1 | temp[i];
        }
        pos.push_back(decimal);
    }
    return pos;
}

class Node{
    public:
        Color color;
        Node *childs[8];
        Node *parent;
    public:
        Node(){
            for(int i=0;i<8;i++){ childs[i] = 0;}
            this->parent = 0;
        }
        Node(Node *parent){
            for(int i=0;i<8;i++){ childs[i] = 0;}
            this->parent = parent;
        }
        ~Node(){};
};

class Octree{
    public:
        Node *root;
        vector<vector<Node*>> levels;
    public:
        Octree(){ 
            this->root = new Node;
            vector<Node*> level;
            for(int i=0;i<8;i++){levels.push_back(level);}
        }
        Octree(Node *root){
            this->root = root;
            vector<Node*> level;
            for(int i=0;i<8;i++){levels.push_back(level);}
        }
        ~Octree(){
            delete root;
            for(int i=0;i<8;i++){
                for(int j=0;j<levels.size();j++){
                    delete levels[i][j];
                }
            }
        }
        
        void insert_color(Color color){
            vector<int> positions = get_position(color);
            Node *temp;
            temp = root;
            for(int i=0;i<positions.size();i++){
                int pos = positions[i];
                if(temp->childs[pos]==0){
                    //temp->childs[pos] = new Node(temp);
                    temp->childs[pos] = new Node;
                    levels[i].push_back(temp->childs[pos]);
                }
                temp->childs[pos]->color = color;
                temp = temp->childs[pos];
            }
        }

        Node* color_reduced(Color color,int deep){
            vector<int> positions = get_position(color);
            Node *temp;
            temp = root;
            for(int i=0;i<deep;i++){
                int pos = positions[i];
                temp = temp->childs[pos];
            }
            return temp;
        }

        CImg<unsigned char> reduce_colors(CImg<unsigned char> image,int deep){
            unsigned char r,g,b;
            CImg<int> img(image.width(),image.height(),1,3);
            for(int x=0;x<img.width();x++){
                for(int y=0;y<img.height();y++){
                    r=image(x,y,0,0);
                    g=image(x,y,0,1);
                    b=image(x,y,0,2);
                    Color color(r,g,b);
                    if(deep <= 0){
                        vector<int> pos = get_position(color);
                        if(pos[0]==0){
                            img(x,y,0,0) = 0;
                        }
                        else{
                            img(x,y,0,0) = 255;
                            img(x,y,0,1) = 255;
                            img(x,y,0,2) = 255;
                        }
                    }
                    else{
                        Node *index_color = color_reduced(color,deep);
                        img(x,y,0,0) = (index_color->color.red);
                        img(x,y,0,1) = (index_color->color.green);
                        img(x,y,0,2) = (index_color->color.blue);
                    }

                }
            }
            return img;
        }

        CImg<unsigned char> make_palette(int n){
            if(n == 0){
                CImg<int> img(1,2,1,3);
                img(0,0,0,0) = 255;
                img(0,0,0,1) = 255;
                img(0,0,0,2) = 255;
                img(0,1,0,0) = 0;
                return img;
            }
            else{
                n--;
                int i = 0;
                int max = levels[n].size();
                int edge = sqrt(max)+1;
                unsigned char r,g,b;
                CImg<int> img(edge,edge,1,3);
                for(int x=0;x<img.width();x++){
                    for(int y=0;y<img.height();y++){
                        if(i < max ){
                            img(x,y,0,0) = levels[n][i]->color.red;
                            img(x,y,0,1) = levels[n][i]->color.green;
                            img(x,y,0,2) = levels[n][i]->color.blue;
                            i++;
                        }
                        else{
                            img(x,y,0,0) = 0;
                            img(x,y,0,1) = 0;
                            img(x,y,0,2) = 0;
                        }
                    }
                }
                return img;
            } 
        }
};

CImg<unsigned char> read_image(Octree *&o,char const *image){
    CImg<unsigned char> img(image);
    unsigned char r,g,b;
    for(int x=0;x<img.width();x++){
        for(int y=0;y<img.height();y++){
            r=img(x,y,0,0);
            g=img(x,y,0,1);
            b=img(x,y,0,2);
            Color color(r,g,b);
            o->insert_color(color);
        }
    }
    return img;
}

int  main(){

    Octree *quantizer = new Octree();
    
    CImg<unsigned char> img;
    CImg<unsigned char> reduced;
    CImg<unsigned char> palette;
    
    int n;  // = 0,1,2,3,4,5,6,7,8  #deep == 0 black & white
    cout<<"deep: ";
    cin>>n;
    img = read_image(quantizer,"snake.jpg");

    reduced = quantizer->reduce_colors(img,n);
    palette = quantizer->make_palette(n); 

    reduced.save("reduced.png");
    palette.save("palette.png");

    reduced.display("Reduced");
    palette.display("Palette");

    delete quantizer;
    
    return 0;
}