package main

import (
	"context"
	"fmt"
	"log"
	"time"

	"github.com/cenkalti/backoff/v5"
	"github.com/jackc/pgx/v5/pgxpool"
)

func ConnectToDB(tomlConfig TomlConfig, ctx *context.Context) *pgxpool.Pool {
	var pool *pgxpool.Pool

	dbURL := fmt.Sprintf(
		"postgres://%s:%s@%s/%s",
		tomlConfig.DbUsername,
		tomlConfig.DbPassword,
		tomlConfig.DbAddress,
		tomlConfig.DbName)

	poolConfig, err := pgxpool.ParseConfig(dbURL)
	if err != nil {
		log.Fatal(err)
	}

	dbConnect := func() (*pgxpool.Pool, error) {
		return pgxpool.NewWithConfig(*ctx, poolConfig)
	}

	expBackoff := backoff.WithBackOff(&backoff.ExponentialBackOff{
		InitialInterval:     time.Millisecond * time.Duration(tomlConfig.DbBackOffInitialInterval),
		RandomizationFactor: tomlConfig.DbBackOffRandomizationFactor,
		Multiplier:          tomlConfig.DbBackOffMultiplier,
		MaxInterval:         time.Second * time.Duration(tomlConfig.DbBackOffMaxInterval),
	})

	pool, err = backoff.Retry(*ctx, dbConnect, expBackoff)
	if err != nil {
		log.Fatal(err)
	}

	return pool
}
