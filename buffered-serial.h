#ifndef BUFFEREDSERIAL_H
#define BUFFEREDSERIAL_H

#include <linux-serial.h>
#include "FixedQueue.h"

#define MAX_PACKET_SIZE 512

class BufferedEscapedLinuxSerialWrapper : public LinuxSerial {

  protected:
    FixedQueue<MAX_PACKET_SIZE> q;   
 
  public:
    static   const uint8_t BEG = 0x12;
    static const uint8_t END = 0x13;
    static const uint8_t DLE = 0x7D;

    
    BufferedEscapedLinuxSerialWrapper() : LinuxSerial() {
  //      _dev = &dev;
        inPacket = false;
        isPacket=false;
        q.clear();
    };
    

    int peek(){ return q.isEmpty() ? -1 : q.peek(); }
    
    int read(){ return q.isEmpty() ? -1 : q.dequeue(); }

    int read(uint8_t * buffer,uint16_t len){
        uint16_t m=len;
        while(!q.isEmpty() && len >=1){
            buffer[m-len] = q.dequeue();
            printf("=>%x\n",buffer[m-len]);
            len--;
        }
        if (m > len && q.isEmpty())
            inPacket = false;
            
        return m-len;
    }
    
    int available() {
      int c;

      if (isPacket &! q.isEmpty())
        return q.count();
        
      if (!LinuxSerial::available())
            return 0;

        c = (uint8_t)LinuxSerial::read();
        if (q.isFull()){
            inPacket = false;
            q.clear();
        }
        
        switch ((uint8_t)c){ 
            case DLE:
                if (inPacket)
                    q.enqueue((uint8_t)LinuxSerial::read());
                break;
            case BEG:
                q.clear();
                inPacket = true;
                isPacket = false;
                break;
            case END:
                if (q.count() >0) {
                    isPacket = true;
                } else {
                    q.clear();
                    inPacket = isPacket = false;
                }
                break;
            default:
                if (inPacket)
                    q.enqueue((uint8_t)c);
                break;
        }
        return (isPacket &! q.isEmpty()) ? q.count() : 0;
    }

    int available(uint16_t c) {
        return c <= available();
    }
    
    size_t write(uint8_t val) {
        //if (!ready())
          // return 0; 
        return LinuxSerial::write(val);
    };  // Call a function on the stored object pointer
    
    int write(const uint8_t * data, int len) {
        uint8_t * _data = (uint8_t*)data;
        int  written = len;
        len=0;
        while(written){
           len++;
           written -= this->write(*_data++);
            }
        return len;
    }
        
    
    void beginPacket() {
        write(BEG);
    }

    void endPacket() {
        write(END);
        this->flush();
    }

  private:
    bool isPacket;
    bool inPacket;
    
    static bool isControl(uint8_t c){
        return (c == DLE || c == BEG || c == END);
    }
};





#endif
