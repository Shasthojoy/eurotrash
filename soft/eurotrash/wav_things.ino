/*
*       
* some things to deal with the wav files. mainly opening + swapping the audio objects
*
*/

void init_channels(uint8_t f) {
  
  uint8_t _file = f;
  for (int i = 0; i < CHANNELS; i ++) {
        
        audioChannels[i]->id = i;
        audioChannels[i]->file_wav = _file;
        audioChannels[i]->pos0 = 0;
        audioChannels[i]->pos1 = CTRL_RESOLUTION;
        audioChannels[i]->ctrl_res = CTRL_RES[_file];
        audioChannels[i]->ctrl_res_eof = CTRL_RES_EOF[_file];
        audioChannels[i]->eof = CTRL_RESOLUTION * CTRL_RES_EOF[_file];
        audioChannels[i]->_gain = DEFAULT_GAIN;  
        audioChannels[i]->swap = false;
  } 
}  

/* =============================================== */

void play_x(uint8_t _channel) {
  
      uint8_t _swap, _select;
      _swap   = audioChannels[_channel]->swap;                
      _select = (_channel*CHANNELS) + _swap;                 // select audio object # 1,2 (LEFT) or 3,4 (RIGHT)
      fade[_select]->fadeIn(FADE_IN);                        // fade in object 1-4
      next_wav(_select, _channel);                           // and play 
  
      /* now swap the file and fade out previous file: */
      _swap = ~_swap & 1u;
      _select = (_channel*CHANNELS) + _swap;
      fade[_select]->fadeOut(FADE_OUT);
      audioChannels[_channel]->swap = _swap;
  
}
 
/* =============================================== */


void next_wav(uint8_t _select, uint8_t _channel) {
  
       /* file */
       uint16_t _file = audioChannels[_channel]->file_wav; 
       /* where to start from? */
       int16_t _CV = (HALFSCALE - CV[3-_channel])>>5;  
       int16_t  _startPos =  _CV + audioChannels[_channel]->pos0; 
       /* limit */
       if (_startPos < 0) _startPos = 0;
       else if (_startPos >= CTRL_RESOLUTION) _startPos = CTRL_RESOLUTION-1;
       /* scale */
       uint32_t _playpos =  _startPos * audioChannels[_channel]->ctrl_res; 
       /* filename */
       String playthis = FILES[_file];  
       /* -> play file X from pos Y */
       wav[_select]->seek(&playthis[0], _playpos>>9); 
       /* now update channel data: */
       audioChannels[_channel]->ctrl_res = CTRL_RES[_file];
       audioChannels[_channel]->ctrl_res_eof = CTRL_RES_EOF[_file];
       
}  

/* =============================================== */

void generate_file_list() {  // to do - sort alphabetically?
  
  uint8_t len;
  uint32_t file_len, file_len_ms;
  char tmp[DISPLAY_LEN];
  File thisfile;
  root = SD.open("/");
  
  thisfile = root.openNextFile(O_RDONLY);  
  while (thisfile && FILECOUNT < MAXFILES) {
              char* _name = thisfile.name(); 
              // wav files ?  
              len = strlen(_name) - 4; 
              if  (!strcmp(&_name[len-2], "~1.WAV")) delay(2); // skip crap
              else if  (_name[0] == '_') delay(2);             // skip crap
              else if (!strcmp(&_name[len], ".WAV")) {
                      
                      FILES[FILECOUNT] = _name;
                      //file_len  = thisfile.size() - 0x2e; // size minus header [ish]
                      /* this is annoying */
                      wav1.play(_name);
                      delay(15);
                      file_len = (float)wav1.lengthBytes() * 0.9f;
                      //FILE_LEN[FILECOUNT]  = file_len;
                      CTRL_RES[FILECOUNT]  = file_len / CTRL_RESOLUTION;       // ctrl resolution pos0/bytes/
                      file_len_ms = wav1.lengthMillis();
                      CTRL_RES_EOF[FILECOUNT] = file_len_ms / CTRL_RESOLUTION; // ctrl resolution pos1/millisec
                      wav1.stop();
                      /* for the display, get rid of .wav extension + right justify */
                      int8_t justify = DISPLAY_LEN - len;
                      if (justify < 0) justify = 0; 
                      for (int i = justify; i < DISPLAY_LEN; i++) {  
                          tmp[i] = _name[i-justify];
                          if (tmp[i] >= 'A' && tmp[i]  <= 'Z' ) tmp[i] = tmp[i] + 'a' - 'A';
                      } 
                      while (justify) {
                          justify--;
                          tmp[justify] = ' '; 
                      }
                      DISPLAYFILES[FILECOUNT] = tmp;
                      FILECOUNT++;
              }    
             thisfile.close();
             thisfile = root.openNextFile(O_RDONLY);
   }   
  root.rewindDirectory(); 
  root.close();
}
  
/* =============================================== */