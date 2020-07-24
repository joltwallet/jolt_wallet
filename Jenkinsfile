pipeline {
    agent any
    environment {
        SSH_KEY = credentials('svc-ops-ssh-key')
        FIREBASE_SERVICE_ACCOUNT = credentials('firebase-adminsdk-ajr72-jolt-wallet')
    }
    stages {
        stage('Build and Push Firmware') {
            when { tag "v*" }
            steps {
                script {
                    echo "Building on Tag: $TAG_NAME"
                    docker.withRegistry('https://index.docker.io/v1/', 'docker-hub-credentials') {
                        sh 'docker pull "joltwallet/jolt_firmware:latest"'
                        sh 'docker run -e SSH_KEY -e TAG_NAME -e FIREBASE_SERVICE_ACCOUNT joltwallet/jolt_firmware:latest'
                    }
                }
            }
        }
        stage('Build Firmware on PR') {
            when {
                branch pattern: "PR-.*", comparator: "REGEXP"
            }
            steps {
                script {
                    echo "Building on PR : $BRANCH_NAME"
                    docker.withRegistry('https://index.docker.io/v1/', 'docker-hub-credentials') {
                        sh 'docker pull "joltwallet/jolt_firmware:latest"'
                        sh 'docker run -e SSH_KEY -e BRANCH_NAME -e FIREBASE_SERVICE_ACCOUNT -e GIT_COMMIT joltwallet/jolt_firmware:latest'
                    }
                }
            }
        }
    }
}

