package main

type Package struct {
	PackageName   string `json:"packageName"`
	VersionString string `json:"versionString"`
}

type PackageList struct {
	Hostname  string    `json:"Hostname"`
	Packages  []Package `json:"packages"`
	Timestamp int64     `json:"timestamp"`
}

type TomlConfig struct {
	DbAddress                    string  `toml:"dbAddress"`
	DbPort                       int     `toml:"dbPort"`
	DbUsername                   string  `toml:"dbUsername"`
	DbPassword                   string  `toml:"dbPassword"`
	DbName                       string  `toml:"dbName"`
	DbBackOffInitialInterval     int     `toml:"dbBackOffInitialInterval"`
	DbBackOffRandomizationFactor float64 `toml:"dbBackOffRandomizationFactor"`
	DbBackOffMultiplier          float64 `toml:"dbBackOffMultiplier"`
	DbBackOffMaxInterval         int     `toml:"dbBackOffMaxInterval"`
	CertFile                     string  `toml:"certFile"`
	KeyFile                      string  `toml:"keyFile"`
	ServerAddress                string  `toml:"serverAddress"`
}
