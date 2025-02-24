# Azeng Programming Language

Azeng adalah bahasa pemrograman sederhana dengan sintaks dalam Bahasa Indonesia.

## Fitur

- Sintaks dalam Bahasa Indonesia
- Tipe data: int, float, bool, string
- Fungsi dengan parameter dan return value
- Kontrol alur: if (kalo) dan while (ulang)
- Variabel dan operasi aritmatika dasar

## Contoh Program

```azeng
fungsi_int tambah(x: int, y: int) {
    kembali x + y;
}

bikin fungsi main() {
    isi hasil = tambah(5, 3);
    cetak("Hasil: ");
    cetak(hasil);
}
```

## Cara Menggunakan

1. Clone repository:
```bash
git clone https://github.com/username/azeng-lang.git
cd azeng-lang
```

2. Build:
```bash
make
```

3. Jalankan program:
```bash
./bin/azeng path/to/program.az
```

## Struktur Proyek

```
azeng/
├── bin/           # Executable files
├── include/       # Header files
│   ├── ast.h      # Abstract Syntax Tree definitions
│   ├── lexer.h    # Lexical analyzer
│   ├── parser.h   # Parser definitions
│   ├── token.h    # Token definitions
│   └── interpreter.h # Interpreter definitions
├── obj/          # Object files
├── src/          # Source files
│   ├── ast.c
│   ├── lexer.c
│   ├── parser.c
│   ├── token.c
│   ├── interpreter.c
│   └── main.c
├── test/         # Test files
│   └── fitur123.az
├── Makefile
└── README.md
```

## Lisensi

MIT License
```
