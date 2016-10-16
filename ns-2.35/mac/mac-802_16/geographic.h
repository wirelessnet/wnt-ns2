#ifndef ns_geographic_h
#define ns_geographic_h
#include "mac-802_16.h"


class Mod{
	public: 
		Mod() : x(-1), y(-1), distance(-1), mod(-1), id(-1), enable(-1), next_(0){}
		double x;
		double y;
		double distance;
		double mod;
		int id;	
		int enable;
		Mod *next_;		
		ModulCodingType ToMod(double bs_x,double bs_y) {
			distance = pow(x-bs_x,2)+pow(y-bs_y,2);
			distance = sqrt(distance);
			printf("SS_X: %f   SS_Y: %f     \nBS_X: %f   BS_Y: %f     distance:%f ",x,y,bs_x,bs_y,distance);
			if (distance/1000 < 10) {
			  	printf(" 64 QAM\n");
				if(distance/1000) return QAM64_34;				  
				return QAM64_23;
			} else if (distance/1000<15) {
				printf(" 16 QAM\n");
				if(distance/1000) return QAM16_34;				  
				return QAM16_12;				   		   
			} else {
				printf(" QPSK\n");
				if(distance/1000) return QPSK_34;				  
				return QPSK_12;	  
			}		
		}
};

class TOMAC {
	private:					
		Mod* head_;
		Mod* tail_;
	public:
		TOMAC() : head_(0), tail_(0){}
		int check(int id_) {			
			for (Mod* tmp = head_; tmp != 0; tmp = tmp->next_) {
				if (tmp->id == id_)
					return (tmp->enable == 1)? 2:1;
			}
			return 0;
		}
		void unlock(int id_) {
			for (Mod* tmp = head_; tmp != 0; tmp = tmp->next_)
				if (tmp->id == id_) tmp->enable = 1;	
		}
		void enque(int id_, double X, double Y) {
			Mod* p = new Mod;
			p->id = id_;
			p->x = X;
			p->y = Y;
			if (!tail_) head_= tail_= p;
			else {
				tail_->next_= p;
				tail_= p;				
			}
			tail_->next_ = 0;				
		}	
		Mod* search(int id_) {
			for (Mod* tmp = head_; tmp != 0; tmp = tmp->next_) {
				if (tmp->id == id_)
					return tmp;
			}
			return 0;		
		}
};

#endif