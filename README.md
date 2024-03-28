# Εξήγηση

Ανεβασμένη είναι η υλοποίηση με το 1 layer με τις παραμέτρους ως σταθερές.

BUF2PE = INBUF2PE. Έχω υποθέσει S=2 και Zero padding=1 στην υλοποίηση, θέλει λίγη τροποποίηση για να αλλάξουν αυτά.

Σε πολλά σημεία έχω ήδη χρησιμοποιήσει την λογική των counter που (λογικά) μεταφράζεται σε FSM.

Αγνοώντας το memory bottleneck του παραδείγματος, (θα διάλεγα μεγαλύτερα unrolling factors), το PE array τροφοδοτείται με δεδομένα σε κάθε κύκλο, με εξαίρεση κάποιους χαμένους λόγω του βάθους του παραλληλισμού. πχ. για Nif=6, Nky=Nkx=4 -> Nif*Nky\*Nkx=96
![syn](https://github.com/aris-gk3/1Layer/assets/82778909/06e4dd54-0196-4a99-9315-4f36b2e7a39a)

Ο compiler του Vitis θέλει για κάθε stream, να έχει module για consumer και producer, το οποίο μπορεί να είναι το ίδιο. Αν δεν γινόταν αυτό, ο compiler τοποθετούσε το stream πιο πάνω στην ιεραρχεία απο μόνος του για να ικανοποιήσει την απαίτηση. Γι'αυτο έβαλα ένα το module INBUF2PE_shell() που περιέχει απλά το INBUF2PE(). Το stream περιέχεται στο INBUF2PE() και σαν producer/consumer βλέπει το INBUF2PE(). (Το stream είναι τα FIFO arrays του paper)

---

# Για να τρέξει

1. Top Layer -> CNN_Layer_top
2. Οι παραμέτροι μπορούν να αλλάξουν απο το "header.h" με τους εξής περιορισμούς
    1. Αν αλλάξει το Tix πρέπει να ενημερωθούν τα constant arrays InBuf_banks και InBuf_rows
    2. Nif, Niy, Nix, Nof, Noy, Nox, Nkx, Nky, Tif, Tiy, Tix, Tof, Toy, Tox, Pof μπορούν να αλλάξουν ελεύθερα, με τον περιορισμό N*%T=0 (modulo)
    3. Για να αλλάξουν τα Poy, Pox χρειάζεται λίγη τροποποίηση στον κώδικα.
3. Έχω βάλει 3 branches στον κώδικα που τεστάρω διαφορετικούς παραμέτρους.