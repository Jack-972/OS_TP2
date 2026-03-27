# Compilateur et options
CC = gcc
# -DTRACE active les messages de débogage demandés à l'étape 3.4
CFLAGS = -Wall -Wextra -g -DTRACE
# Librairie readline pour l'interpréteur biceps
LDFLAGS = -lreadline

# Cibles
TARGET = biceps
OBJ = biceps.o creme.o gescom.o

# Règle par défaut
all: $(TARGET)

# Édition de liens finale
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

# Compilation des fichiers objets
%.o: %.c creme.h gescom.h
	$(CC) $(CFLAGS) -c $<

# Nettoyage
clean:
	rm -f *.o $(TARGET) servbeuip clibeuip

# Vérification des symboles exportés (Etape 3.1)
check: creme.o
	nm creme.o
	objdump -t creme.o