# TP4_REINGL_DSY_BSH

## Correction du design
### Dans la schématique MCU
Dans cette partie du schéma, nous avions constaté plusieurs erreurs. La première était les condensateurs du quartz externe : nous avions oublié de changer leur valeur. Leurs valeurs passent donc de 100 nF à 22 pF.

Voici le changement :

<img width="577" height="354" alt="clock apres changement" src="https://github.com/user-attachments/assets/17e445ff-1243-4a20-8897-d94d779be5d2" />


Lors de la programmation, nous avons constaté deux autres erreurs. La première concerne les broches (pins) pour la sonde de debug : celle mise dans la schématique avait trop de broches inutiles. Il y avait aussi le problème que le sens de placement des broches n'était pas pratique avec la sonde, ce qui laissait une pins femelle de la sonde flottante. Voici donc le changement vers une série de broches plus adaptée et conforme à la longueur de la sonde :

<img width="474" height="429" alt="Pics pour debug apreschangement" src="https://github.com/user-attachments/assets/7478b509-5bd2-467e-9d4d-b7a44c2a78a9" />

La deuxième erreur constatée lors de la programmation est une erreur de mapping : nous avions mis la consigne reçue par le "backplane" sur une broche du microcontrôleur n'ayant aucun ADC. Nous avons donc dû faire une manipulation pour mettre la consigne reçue sur la broche 23 disposant d'un ADC (AN12) :

<img width="934" height="485" alt="MCU apres changement" src="https://github.com/user-attachments/assets/e54b8078-3160-43ab-a470-a65f28c3cbb5" />


### Dans la schématique MOSFET_Driver_block
Dans cette schématique, nous avons fait une erreur simple : le calcul pour le dimensionnement des résistances de grille des MOSFET n'avait pas été fait. Voici donc la correction avec les calculs :

<img width="376" height="161" alt="image" src="https://github.com/user-attachments/assets/100944c8-d936-4422-8918-f22a25feb193" />


Avec ces informations tirées de leurs datasheets respectives pour calculer le courant dans la grille (gate) :

Tiré de la datasheet du MOSFET DMN6068LK3-13 :

<img width="617" height="513" alt="image" src="https://github.com/user-attachments/assets/c3c979a0-bf2a-449e-a9ca-ea3e8f51084f" />

  

<img width="527" height="19" alt="image" src="https://github.com/user-attachments/assets/cd25a7c8-646e-4f76-8c8a-8fa9ed5f65bd" />


Tiré de la datasheet du driver de MOSFET IR2184(4)(S)&(PBF) :


<img width="450" height="22" alt="image" src="https://github.com/user-attachments/assets/124f861b-1f18-4494-a67c-849cc6f1d910" />


Nous trouvons donc $30\ \Omega$ pour ces résistances de grille.

















