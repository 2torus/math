#!/usr/bin/env groovy
@Library('StanUtils')
import org.stan.Utils

def setupCC(Boolean failOnError = true) {
    errorStr = failOnError ? "-Werror " : ""
    "echo CC=${env.CXX} ${errorStr}> make/local"
}

def setup(Boolean failOnError = true) {
    sh """
        git clean -xffd
        ${setupCC(failOnError)}
    """
}

def mailBuildResults(String label, additionalEmails='') {
    emailext (
        subject: "[StanJenkins] ${label}: Job '${env.JOB_NAME} [${env.BUILD_NUMBER}]'",
        body: """${label}: Job '${env.JOB_NAME} [${env.BUILD_NUMBER}]': Check console output at ${env.BUILD_URL}""",
        recipientProviders: [[$class: 'RequesterRecipientProvider']],
        to: "${env.CHANGE_AUTHOR_EMAIL}, ${additionalEmails}"
    )
}

def runTests(String testPath) {
    sh "./runTests.py -j${env.PARALLEL} ${testPath} --make-only"
    try { sh "./runTests.py -j${env.PARALLEL} ${testPath}" }
    finally { junit 'test/**/*.xml' }
}

def utils = new org.stan.Utils()

def isBranch(String b) { env.BRANCH_NAME == b }

def alsoNotify() {
    if (isBranch('master') || isBranch('develop')) {
        "stan-buildbot@googlegroups.com"
    } else ""
}
def mathUrl() { "https://github.com/stan-dev/math" }
def isFork() { env.CHANGE_URL && !env.CHANGE_URL.startsWith(mathUrl()) }
def branchName() {
    if (env.CHANGE_BRANCH) {
        br = env.CHANGE_BRANCH
        if (isFork()) {
            br = "autoformat/" + br
        }
    } else {
        br = env.BRANCH_NAME
    }
    return br
}
def remoteName() { isFork() ? mathUrl() : "origin" }
def force() { isFork() ? "-f" : "" }

pipeline {
    agent none
    parameters {
        string(defaultValue: 'downstream tests', name: 'cmdstan_pr',
          description: 'PR to test CmdStan upstream against e.g. PR-630')
        string(defaultValue: 'downstream tests', name: 'stan_pr',
          description: 'PR to test Stan upstream against e.g. PR-630')
        booleanParam(defaultValue: false, description:
        'Run additional distribution tests on RowVectors (takes 5x as long)',
        name: 'withRowVector')
    }
    options { skipDefaultCheckout() }
    stages {
        stage('Kill previous builds') {
            when {
                not { branch 'develop' }
                not { branch 'master' }
            }
            steps { 
                script {
                    utils.killOldBuilds()
                }
            }
        }
        stage("Clang-format") {
            agent any
            steps {
                sh "printenv"
                retry(3) { checkout scm }
                withCredentials([usernamePassword(credentialsId: 'a630aebc-6861-4e69-b497-fd7f496ec46b',
                    usernameVariable: 'GIT_USERNAME', passwordVariable: 'GIT_PASSWORD')]) {
                    sh """#!/bin/bash
                        set -x
                        git checkout -b ${branchName()}
                        clang-format --version
                        find stan test -name '*.hpp' -o -name '*.cpp' | xargs -n20 -P${env.PARALLEL} clang-format -i
                        if [[ `git diff` != "" ]]; then
                            git config --global user.email "mc.stanislaw@gmail.com"
                            git config --global user.name "Stan Jenkins"
                            git add stan test
                            git commit -m "[Jenkins] auto-formatting by `clang-format --version`"
                            git push ${force()} https://${GIT_USERNAME}:${GIT_PASSWORD}@github.com/stan-dev/math.git ${branchName()} 
                            echo "Exiting build because clang-format found changes."
                            echo "Those changes are now found on stan-dev/math under branch ${branchName()}"
                            echo "Please 'git pull ${remoteName()} ${branchName()}' before continuing to develop."
                            exit 1
                        fi"""
                }
            }
            post {
                always { deleteDir() }
                failure {
                    script {
                        if (isFork()) {
                            emailext (
                                subject: "[StanJenkins] Autoformattted: Job '${env.JOB_NAME} [${env.BUILD_NUMBER}]'",
                                body: """
                                Job '${env.JOB_NAME} [${env.BUILD_NUMBER}]'
                                has been autoformatted and the changes committed to
                                ${remoteName()} on branch '${branchName()}'
                                Please pull these changes before continuing. 

                                See https://github.com/stan-dev/stan/wiki/Coding-Style-and-Idioms
                                for local development environment setup.

                                (Check console output at ${env.BUILD_URL})""",
                                recipientProviders: [[$class: 'RequesterRecipientProvider']],
                                to: "${env.CHANGE_AUTHOR_EMAIL}"
                            )
                        }
                    }
                }
            }
        }
        stage('Linting & Doc checks') {
            agent any
            steps {
                script {
                    retry(3) { checkout scm }
                    setup(false)
                    stash 'MathSetup'
                    sh setupCC()
                    parallel(
                        CppLint: { sh "make cpplint" },
                        Dependencies: { sh 'make test-math-dependencies' } ,
                        Documentation: { sh 'make doxygen' },
                        Headers: { sh "make -j${env.PARALLEL} test-headers" }
                    )
                }
            }
            post {
                always {
                    warnings consoleParsers: [[parserName: 'CppLint']], canRunOnFailed: true
                    warnings consoleParsers: [[parserName: 'math-dependencies']], canRunOnFailed: true
                    deleteDir()
                }
            }
        }
        stage('Tests') {
            parallel {
                stage('Unit') {
                    agent any
                    steps {
                        unstash 'MathSetup'
                        sh setupCC()
                        runTests("test/unit")
                    }
                    post { always { retry(3) { deleteDir() } } }
                }
                stage('Distribution tests') {
                    agent { label "distribution-tests" }
                    steps { 
                        unstash 'MathSetup'
                        sh """
                            ${setupCC(false)}
                            echo 'O=0' >> make/local
                            echo N_TESTS=${env.N_TESTS} >> make/local
                            """
                        script {
                            if (params.withRowVector || isBranch('develop') || isBranch('master')) {
                                sh "echo CXXFLAGS+=-DSTAN_TEST_ROW_VECTORS >> make/local"
                            }
                        }
                        sh "./runTests.py -j${env.PARALLEL} test/prob > dist.log 2>&1"
                    }
                    post {
                        always {
                            script { zip zipFile: "dist.log.zip", archive: true, glob: 'dist.log' }
                            retry(3) { deleteDir() }
                        }
                        failure {
                            echo "Distribution tests failed. Check out dist.log.zip artifact for test logs."
                        }
                    }
                }
            }
        }
        stage('Upstream tests') {
            parallel {
                stage('CmdStan Upstream Tests') {
                    when { expression { env.BRANCH_NAME ==~ /PR-\d+/ } }
                    steps {
                        build(job: "CmdStan/${params.cmdstan_pr}",
                                    parameters: [string(name: 'math_pr', value: env.BRANCH_NAME)])
                    }
                }
                stage('Stan Upstream Tests') {
                    when { expression { env.BRANCH_NAME ==~ /PR-\d+/ } }
                    steps {
                        build(job: "Stan/${params.stan_pr}",
                                    parameters: [string(name: 'math_pr', value: env.BRANCH_NAME)])
                    }
                }
            }
        }
    }
    post {
        always {
            node("osx || linux") {
                warnings consoleParsers: [[parserName: 'GNU C Compiler 4 (gcc)']], canRunOnFailed: true
                warnings consoleParsers: [[parserName: 'Clang (LLVM based)']], canRunOnFailed: true
            }
        }
        success {
            script { utils.updateUpstream(env, 'stan') }
            mailBuildResults("SUCCESSFUL")
        }
        unstable { mailBuildResults("UNSTABLE", alsoNotify()) }
        failure { mailBuildResults("FAILURE", alsoNotify()) }
    }
}