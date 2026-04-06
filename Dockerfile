FROM gcc:12-bookworm AS builder

WORKDIR /app

COPY mainV5.cpp .

RUN g++ -O3 -flto -funroll-loops -pthread mainV5.cpp -o main

FROM debian:bookworm-slim

WORKDIR /app

COPY --from=builder /app/main /app/main

RUN chmod +x /app/main

ENTRYPOINT ["/app/main"]