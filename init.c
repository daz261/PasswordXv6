// init: The initial user-level program

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "bcrypt.h"
#define MAX 128

char *argv[] = { "sh", 0 };

void setPassword(char* entered_pass) {
  int j = 0;
  uchar salt[BCRYPT_SALTLEN];
  int fd;

  for( j = 0; j < 16; ++j ){
        salt[j] = random();
       
        if ( (fd = open("passwd", O_CREATE | O_WRONLY)) > 0 )
        {
          write(fd, salt, BCRYPT_SALTLEN);
         // printf(1, "salt: %d\n", salt );
         // printf(1, "bcrypt: %s\n", bcrypt( entered_pass, (uchar *) salt ) );
          write(fd, bcrypt( entered_pass, (uchar *) salt ), BCRYPT_HASHLEN);
          close(fd);
        } 
        else {
           exit();
        } 
    }

}

int
main(void)
{
  int pid, wpid;

  if(open("console", O_RDWR) < 0){
    mknod("console", 1, 1);
    open("console", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

  char entered_pass[MAX];
  char verified_pass[MAX];

  int fd;
  int i = 0;

  uchar salt[BCRYPT_SALTLEN];
  uchar goodhash[BCRYPT_HASHLEN];

  if ( (fd = open("passwd", O_RDONLY)) < 0 ) {
    printf(1, "No password set. Please choose one.\nEnter password: ");
    gets(entered_pass, MAX);
    //printf(1, "%d", entered_pass);
    printf(1, "Retype the password: ");
    gets(verified_pass, MAX);
   // printf(1, "%d", verified_pass);

   for( i = 0; i < MAX; ++i ) {
    if (entered_pass[i] != verified_pass[i]) 
      {

         while ( entered_pass[i] != verified_pass[i]){
            printf(1, "Passwords do not match. Try again.");
            printf(1, "Enter password: ");
            gets(entered_pass, MAX);

            printf(1, "Retype the password: ");
            gets(verified_pass, MAX);
           

            if (entered_pass == verified_pass) {
           //     printf(1, "Password successfully set.");
                setPassword(entered_pass);
                printf(1, "Password successfully set. You may now use it to log in.\n");

                fd = open("passwd", O_RDONLY);
                printf(1, "Enter password: ");
                gets(entered_pass, MAX);

                read(fd, salt, sizeof(salt));
                read(fd, goodhash, sizeof(goodhash));
                if ( read(fd, salt, sizeof(salt)) < 0)
                      exit();
                if ( read(fd, goodhash, sizeof(goodhash)) < 0 )
                      exit();

                while ( bcrypt_checkpass(entered_pass, (uchar *) salt, goodhash) < 0 ) {
                  printf(1, "Passwords do not match. Try again.");
                  gets(entered_pass, sizeof(entered_pass));
                  if (bcrypt_checkpass(entered_pass, (uchar *) salt, goodhash) > 0 ){
                        printf(1, "Password correct, logging you in.");
                        break;
                      }
                } 
                close(fd); 
                  // break;
            }//end else
            
            break;
            } //end while

        }

     // }  //end if

      else if (entered_pass[i] == verified_pass[i]) 
      {

        setPassword(entered_pass);
        printf(1, "Password successfully set. You may now use it to log in.\n");

        fd = open("passwd", O_RDONLY);
        printf(1, "Enter password: ");
        gets(entered_pass, MAX);

        read(fd, salt, sizeof(salt));
        read(fd, goodhash, sizeof(goodhash));
        if ( read(fd, salt, sizeof(salt)) < 0)
              exit();
        if ( read(fd, goodhash, sizeof(goodhash)) < 0 )
              exit();

        while ( bcrypt_checkpass(entered_pass, (uchar *) salt, goodhash) < 0 ) {
          printf(1, "Passwords do not match. Try again.");
          printf(1, "Enter password: ");
          gets(entered_pass, sizeof(entered_pass));
          if (bcrypt_checkpass(entered_pass, (uchar *) salt, goodhash) > 0 ){
                printf(1, "Password correct, logging you in.");
                break;
              }
        } 
        close(fd); 
       break;
      }//end else
    
}
   //old FOR }
  }//end if  

else {
    fd = open("passwd", O_RDONLY);
    printf(1, "Enter password: ");
    gets(entered_pass, MAX);

    read(fd, salt, sizeof(salt));
    read(fd, goodhash, sizeof(goodhash));
    if ( read(fd, salt, sizeof(salt)) < 0)
        exit();
    if ( read(fd, goodhash, sizeof(goodhash)) < 0 )
        exit();

    while ( bcrypt_checkpass(entered_pass, (uchar *) salt, goodhash) < 0 ) {
      printf(1, "Invalid attempt, re-enter password: ");
      gets(entered_pass, sizeof(entered_pass));
      if (bcrypt_checkpass(entered_pass, (uchar *) salt, goodhash) > 0 ){
            printf(1, "Password correct, logging you in.");
            break;
          }
    } 
   close(fd); 
  }
 

  for(;;){
    printf(1, "init: starting sh\n");
    pid = fork();
    if(pid < 0){
      printf(1, "init: fork failed\n");
      exit();
    }
    if(pid == 0){
      exec("sh", argv);
      printf(1, "init: exec sh failed\n");
      exit();
    }
    while((wpid=wait()) >= 0 && wpid != pid)
      printf(1, "zombie!\n");
  }
}
