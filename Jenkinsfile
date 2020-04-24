pipeline {
    agent any
    environment {
        SSH_KEY = credentials('svc-ops-ssh-key')
        GOOGLE_APPLICATION_CREDENTIALS = credentials('firebase-adminsdk-ajr72-jolt-wallet')
    }
    stages {
        stage('Build Firmware on Tag') {
            when { tag "v*" }
            steps {
                script {
                    echo "Building $TAG_NAME"
                    docker.withRegistry('https://index.docker.io/v1/', 'docker-hub-credentials') {
                        sh 'docker pull "joltwallet/jolt_firmware:latest"'
                        sh 'docker run -e SSH_KEY -e TAG_NAME joltwallet/jolt_firmware:latest'
                    }
                }
            }
        }
        stage('TEST - Build Firmware') {
            steps {
                script {
                    echo "Building $BRANCH_NAME"
                    docker.withRegistry('https://index.docker.io/v1/', 'docker-hub-credentials') {
                        sh 'docker pull "joltwallet/jolt_firmware:latest"'
                        sh 'docker run -e SSH_KEY -e TAG_NAME=${BRANCH_NAME} -e GOOGLE_APPLICATION_CREDENTIALS joltwallet/jolt_firmware:latest'
                    }
                }
            }
        }
    }
}

