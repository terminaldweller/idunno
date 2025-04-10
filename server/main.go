package main

import (
	"context"
	"flag"
	"log"
	"net/http"
	"os"
	"time"

	"github.com/BurntSushi/toml"
	"github.com/jackc/pgx/v5/pgxpool"
)

type apiHandler struct {
	pool *pgxpool.Pool
}

func (h *apiHandler) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	w.Write([]byte("hello"))
}

func main() {

	configFilePath := flag.String("config", "", "config file path")

	flag.Parse()

	data, err := os.ReadFile(*configFilePath)
	if err != nil {
		log.Fatal(err)
	}

	var tomlConfig TomlConfig

	_, err = toml.Decode(string(data), &tomlConfig)

	mux := http.NewServeMux()

	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	pool := ConnectToDB(tomlConfig, &ctx)

	mux.Handle("/api/v1/intake", &apiHandler{pool: pool})

	http.ListenAndServeTLS(tomlConfig.ServerAddress, tomlConfig.CertFile, tomlConfig.KeyFile, mux)
}
