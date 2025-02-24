# Azeng Programming Language

Azeng adalah bahasa pemrograman sederhana dengan sintaks dalam Bahasa Indonesia.

## Fitur

- Sintaks dalam Bahasa Indonesia
- Tipe data: integer, float, string, boolean
- Fungsi dengan parameter dan return value
- Kontrol alur: if-else, loop
- Operasi aritmatika dan perbandingan
- Manipulasi string

## Instalasi

```bash
git clone https://github.com/username/azeng.git
cd azeng
make
```

## Penggunaan

```bash
./bin/azeng program.az
```

## Contoh Program

### Hello World
```azeng
bikin fungsi main() {
    cetak("Halo dunia!");
}
```

### Fungsi dan Tipe Data
```azeng
fungsi_int tambah(x: int, y: int) {
    kembali x + y;
}

bikin fungsi main() {
    isi hasil = tambah(10, 5);
    cetak("Hasil: ");
    cetak(hasil);
}
```

### Loop
```azeng
bikin fungsi main() {
    isi i = 1;
    ulang (i < 5) {
        cetak("Perulangan ke-");
        cetak(i);
        isi i = i + 1;
    }
}
```

## Sintaks Dasar

### Variabel
```azeng
isi x = 10;
isi nama = "John";
isi nilai = 3.14;
isi benar = benar;
```

### Fungsi
```azeng
fungsi_int tambah(x: int, y: int) {
    kembali x + y;
}

fungsi_float bagi(a: float, b: float) {
    kembali a / b;
}

fungsi_bool lebih_besar(n1: int, n2: int) {
    kembali n1 > n2;
}

fungsi_str gabung(kata1: str, kata2: str) {
    kembali kata1 + kata2;
}
```

### Kontrol Alur
```azeng
kalo (x > 5) {
    cetak("x lebih dari 5");
}

ulang (i < 10) {
    cetak(i);
    isi i = i + 1;
}
```

## Lisensi

MIT License 