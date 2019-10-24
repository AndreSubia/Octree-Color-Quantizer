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
            this->red = 0;
            this->green = 0;
            this->blue = 0;
        }
        Color(int r,int g, int b){
            this->red = r;
            this->green = g;
            this->blue = b;
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
        int lvl;
        int pixel_count;
        Node *childs[8];
        Node *parent;
    public:
        Node(){
            for(int i=0;i<8;i++){ childs[i] = 0;}
            this->pixel_count = 0;
            this->parent = 0;
        };

        Node(int lvl,Node *parent){
            for(int i=0;i<8;i++){ childs[i] = 0;}
            this->lvl = lvl;
            this->pixel_count = 0;
            this->parent = parent;
        }

        ~Node(){}
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
        ~Octree(){}
        
        void insert_color(Color color){
            vector<int> positions = get_position(color);
            Node *temp;
            temp = root;
            for(int i=0;i<positions.size();i++){
                int pos = positions[i];
                if(temp->childs[pos]==0){
                    temp->childs[pos] = new Node(i,temp);
                    levels[i].push_back(temp->childs[pos]);
                }
                if( i==positions.size()-1 && temp!=0 ){
                    temp->childs[pos]->pixel_count++;
                    Color ncolor(temp->childs[pos]->color.red + color.red,
                                        temp->childs[pos]->color.green + color.green,
                                        temp->childs[pos]->color.blue + color.blue);
                    temp->childs[pos]->color = ncolor;
                    return;
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
                        Node *index_color = color_reduced(color,deep);
                        img(x,y,0,0) = (index_color->color.red);
                        img(x,y,0,1) = (index_color->color.green);
                        img(x,y,0,2) = (index_color->color.blue);
                }
            }
            return img;
        }

        CImg<unsigned char> make_palette(int n){
            n = n - 1;
            int i = 0;
            int max = levels[n].size();
            int edge = sqrt(max)+1;
            unsigned char r,g,b;
            CImg<int> img(edge,edge,1,3);
            for(int x=0;x<img.width();x++){
                for(int y=0;y<img.height();y++){
                    if(i < max ){
                        int pixel_c = levels[n][i]->pixel_count;
                        if(pixel_c == 0){pixel_c = 1;}
                        img(x,y,0,0) = (levels[n][i]->color.red/pixel_c);
                        img(x,y,0,1) = (levels[n][i]->color.green/pixel_c);
                        img(x,y,0,2) = (levels[n][i]->color.blue/pixel_c);
                    }
                    else{
                        img(x,y,0,0) = 0;
                        img(x,y,0,1) = 0;
                        img(x,y,0,2) = 0;
                    }
                    i++;
                }
            }
            return img;
        }
};

CImg<unsigned char> read_image(Octree *&o,char const *title){
    CImg<unsigned char> image(title);
    unsigned char r,g,b;
    for(int x=0;x<image.width();x++){
        for(int y=0;y<image.height();y++){
                r=image(x,y,0,0);
                g=image(x,y,0,1);
                b=image(x,y,0,2);
                Color color(r,g,b);
                o->insert_color(color);
        }
    }
    return image;
}

int  main(){

    Octree *quantizer = new Octree();
    CImg<unsigned char> img;
    CImg<unsigned char> reduced;
    CImg<unsigned char> palette;
    
    int n = 3;
    img = read_image(quantizer,"rainbow.jpg");

    palette = quantizer->make_palette(n); 
    reduced = quantizer->reduce_colors(img,n);

    reduced.save("reduced.png");
    palette.save("palette.png");

    reduced.display("Reduced");
    palette.display("Palette");

    free(quantizer);
    return 0;
}
