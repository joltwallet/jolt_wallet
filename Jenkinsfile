pipeline {
    agent any
    environment {
        SSH_KEY = credentials('svc-ops-ssh-key')
    }
    stages {
        stage('Build Firmware on Tag') {
            when { tag "v*" }
            steps {
                script {
                    docker.withRegistry('https://index.docker.io/v1/', 'docker-hub-credentials') {
                        sh 'docker pull "joltwallet/jolt_firmware:latest"'
                        sh 'docker run -e SSH_KEY joltwallet/jolt_firmware:latest'
                    }
                }
            }
        }
        stage('TEST - Build Firmware') {
            steps {
                script {
                    docker.withRegistry('https://index.docker.io/v1/', 'docker-hub-credentials') {
                        sh 'docker pull "joltwallet/jolt_firmware:latest"'
                        sh 'docker run -e SSH_KEY joltwallet/jolt_firmware:latest'
                    }
                }
            }
        }
    }
}